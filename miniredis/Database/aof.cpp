#include "aof.h"

unordered_set<string_view> cmd_need_aof = {
    "set",   "incr",    "decr", "append", // string command
    "hset",  "hdel",                      // hash command
    "lpush", "rpush",   "lpop", "rpop",   // list command
    "sadd",  "srem",                      // set command
    "zadd",  "zrem",                      // zset command
    "del",   "flushall"                   // general command
};

Aof::Aof(asio::thread_pool& work_executor, asio::io_context& io_context)
    : work_executor(work_executor), io_context(io_context), aof_timer(io_context)
{
}

Aof::~Aof()
{
}

awaitable<void> Aof::aofTimerHandler()
{
    while (true)
    {
        aof_timer.expires_after(std::chrono::seconds(AOF_TIMER_INTERVAL));
        co_await aof_timer.async_wait(use_awaitable);

        std::cout << "simpifying AOF" << std::endl;
        simplifyAOF();
        std::cout << "simpify AOF done" << std::endl;

        std::cout << "storing AOF" << std::endl;
        storeAOF("aof.dat");
        std::cout << "store AOF done" << std::endl;

        std::cout << "compressing AOF" << std::endl;
        CompressFileStream("aof.dat", "aof.dat.gz");
        std::cout << "compress AOF done" << std::endl;

        // destroy aof_cmds
        for (auto& cmd : aof_cmds)
            for (auto& sds : cmd)
                Sds::destroy(sds);
        aof_cmds.clear();
    }
}

void Aof::startAof()
{
    co_spawn(work_executor, aofTimerHandler(), detached);
}

void Aof::simplifyAOF()
{
    int size = aof_cmds.size();
    for (int index = 0; index < size; index++)
    {
        Command& cmd = aof_cmds[index];

        if (!simplifyStringCommand(index, cmd) && !simplifyListCommand(index, cmd) &&
            !simplifyHashCommand(index, cmd) && !simplifySetCommand(index, cmd) &&
            !simplifyZSetCommand(index, cmd) && !simplifyGeneralCommand(index, cmd))
        {
            temp_general_cmds.emplace_back(index, cmd);
        }
    }

    // delete redundant commands
    unordered_set<int> remained_cmds_index;
    for (auto& [key, value] : temp_cmds)
    {
        for (auto& [index, cmd] : value)
        {
            remained_cmds_index.insert(index);
        }
    }
    for (auto& [index, cmd] : temp_general_cmds)
    {
        remained_cmds_index.insert(index);
    }

    vector<Command> simplified_cmds;
    for (int index = 0; index < size; index++)
    {
        if (!remained_cmds_index.contains(index))
        {
            for (auto& sds : aof_cmds[index])
            {
                Sds::destroy(sds);
            }
        }
        else
        {
            simplified_cmds.push_back(aof_cmds[index]);
        }
    }

    // update aof_cmds
    aof_cmds = simplified_cmds;

    // temp container clear
    temp_cmds.clear();
    temp_general_cmds.clear();
}

void Aof::addCmdToAof(Command& cmd)
{
    aof_cmds.push_back(cmd);
}

void Aof::loadAOF(const string& path)
{
    ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs)
        throw std::runtime_error("open aof file failed");

    auto expect_size = struct_pack::deserialize<int>(ifs);
    if (!expect_size)
        throw std::runtime_error("read aof file failed");

    int size = expect_size.value();
    for (int i = 0; i < size; ++i)
    {
        auto expect_cmd_size = struct_pack::deserialize<int>(ifs);
        if (!expect_cmd_size)
            throw std::runtime_error("read aof file failed");

        Command cmd;
        int cmd_size = expect_cmd_size.value();
        for (int j = 0; j < cmd_size; ++j)
            cmd.push_back(Sds::deserialize_from(ifs));
        aof_cmds.push_back(cmd);
    }
}

void Aof::storeAOF(const string& path)
{
    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    ofstream ofs(path, std::ios::out | std::ios::binary);
    if (!ofs)
        throw std::runtime_error("open aof file failed");

    int size = aof_cmds.size();
    struct_pack::serialize_to(ofs, size);
    for (const auto& cmd : aof_cmds)
    {
        int cmd_size = cmd.size();
        struct_pack::serialize_to(ofs, cmd_size);
        for (const auto& sds : cmd)
            Sds::serialize_to(ofs, sds);
    }
}

bool Aof::simplifyStringCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    // string command simplify
    if (cmd_type->strcmp("set") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        list.clear();
        list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("incr") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("set") == 0)
        {
            Command& set_cmd = list.back().second;
            Sds* value = set_cmd[2];

            int64_t value_int = sds2num<int64_t>(value).value();
            Sds* incr_value = num2sds(value_int + 1);
            set_cmd[2] = incr_value;
            Sds::destroy(value);
        }
        else
        {
            list.emplace_back(index, cmd);
        }
    }
    else if (cmd_type->strcmp("decr") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("set") == 0)
        {
            Command& set_cmd = list.front().second;
            Sds* value = set_cmd[2];

            int64_t value_int = sds2num<int64_t>(value).value();
            Sds* decr_value = num2sds(value_int - 1);
            set_cmd[2] = decr_value;
            Sds::destroy(value);
        }
        else
        {
            list.emplace_back(index, cmd);
        }
    }
    else if (cmd_type->strcmp("append") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("set") == 0)
        {
            Command& set_cmd = list.back().second;
            set_cmd[2] = set_cmd[2]->append(cmd[2]->buf, cmd[2]->length());
        }
        else
        {
            list.emplace_back(index, cmd);
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::simplifyListCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    if (cmd_type->strcmp("lpush") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        auto it = list.rbegin();
        for (; it != list.rend(); ++it)
        {
            if (it->second[0]->strcmp("lpush") == 0)
            {
                Command& lpush_cmd = it->second;

                lpush_cmd.insert(lpush_cmd.end(), cmd.begin() + 2, cmd.end());
                cmd.erase(cmd.begin() + 2, cmd.end());
                break;
            }
        }
        if (it == list.rend())
            list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("rpush") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        auto it = list.begin();
        for (; it != list.end(); ++it)
        {
            if (it->second[0]->strcmp("rpush") == 0)
            {
                Command& rpush_cmd = it->second;

                rpush_cmd.insert(rpush_cmd.end(), cmd.begin() + 2, cmd.end());
                cmd.erase(cmd.begin() + 2, cmd.end());
                break;
            }
        }
        if (it == list.end())
            list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("lpop") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        auto it = list.rbegin();
        for (; it != list.rend(); ++it)
        {
            if (it->second[0]->strcmp("lpush") == 0)
            {
                break;
            }
        }
        if (it == list.rend())
        {
            list.emplace_back(index, cmd);
            return true;
        }

        Command& lpush_cmd = it->second;
        if (lpush_cmd.size() == 3)
        {
            list.erase(--it.base());
        }
        else
        {
            temp_del_values.push_back(lpush_cmd.back());
            lpush_cmd.pop_back();
        }
    }
    else if (cmd_type->strcmp("rpop") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        auto it = list.rbegin();
        for (; it != list.rend(); ++it)
        {
            if (it->second[0]->strcmp("rpush") == 0)
            {
                break;
            }
        }
        if (it == list.rend())
        {
            list.emplace_back(index, cmd);
            return true;
        }

        Command& rpush_cmd = it->second;
        if (rpush_cmd.size() == 3)
        {
            list.erase(--it.base());
        }
        else
        {
            temp_del_values.push_back(rpush_cmd.back());
            rpush_cmd.pop_back();
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::simplifyHashCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    if (cmd_type->strcmp("hset") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("hset") == 0)
        {
            Command& hset_cmd = list.back().second;

            hset_cmd.insert(hset_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("hdel") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("hdel") == 0)
        {
            Command& hdel_cmd = list.back().second;

            hdel_cmd.insert(hdel_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::simplifySetCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    if (cmd_type->strcmp("sadd") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("sadd") == 0)
        {
            Command& sadd_cmd = list.back().second;

            sadd_cmd.insert(sadd_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("srem") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("srem") == 0)
        {
            Command& srem_cmd = list.back().second;

            srem_cmd.insert(srem_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::simplifyZSetCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    if (cmd_type->strcmp("zadd") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("zadd") == 0)
        {
            Command& zadd_cmd = list.back().second;

            zadd_cmd.insert(zadd_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else if (cmd_type->strcmp("zrem") == 0)
    {
        auto& list = temp_cmds[cmd[1]];

        if (list.size() && list.back().second[0]->strcmp("zrem") == 0)
        {
            Command& zrem_cmd = list.back().second;

            zrem_cmd.insert(zrem_cmd.end(), cmd.begin() + 2, cmd.end());
            cmd.erase(cmd.begin() + 2, cmd.end());
        }
        else
            list.emplace_back(index, cmd);
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::simplifyGeneralCommand(int index, Command& cmd)
{
    Sds* cmd_type = cmd[0];
    if (cmd_type->strcmp("del") == 0)
    {
        int cmd_size = cmd.size();
        for (int i = 1; i < cmd_size; ++i)
        {
            temp_cmds[cmd[i]].clear();
            temp_cmds[cmd[i]].emplace_back(index, cmd);
        }
    }
    else if (cmd_type->strcmp("flushall") == 0)
    {
        temp_cmds.clear();
        temp_general_cmds.emplace_back(index, cmd);
    }
    else
    {
        return false;
    }
    return true;
}

bool Aof::isCmdNeedAof(Sds* cmdtype)
{
    return cmd_need_aof.contains(string_view(cmdtype->buf, cmdtype->length()));
}