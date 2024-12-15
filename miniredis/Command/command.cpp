#include "server.h"
#include "command.h"

std::unordered_map<std::string, std::function<void(shared_ptr<Connection> conn, Command&)>>
    commands_map = {
        // string command
        {"set", CmdSet},
        {"get", CmdGet},
        {"incr", CmdIncr},
        {"decr", CmdDecr},
        {"append", CmdAppend},

        // hash command
        {"hset", CmdHSet},
        {"hget", CmdHGet},
        {"hdel", CmdHDel},
        {"hkeys", CmdHKeys},
        {"hgetall", CmdHGetAll},

        // list command
        {"lpop", CmdLPop},
        {"rpop", CmdRPop},
        {"lpush", CmdLPush},
        {"rpush", CmdRPush},
        {"lrange", CmdLRange},

        // set command
        {"sadd", CmdSAdd},
        {"srem", CmdSRem},
        {"smembers", CmdSMembers},
        {"sismember", CmdSisMember},

        // zset command
        {"zadd", CmdZAdd},
        {"zrem", CmdZRem},
        {"zrange", CmdZRange},
        {"zrevrange", CmdZRevRange},

        // general command
        {"del", CmdDel},
        {"keynum", CmdKeyNum},
        {"flushall", CmdFlushAll},
};

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateErrorReply(const char* errmsg)
{
    Sds* reply = Sds::create("-ERR ", 5, 5);
    reply = reply->append(errmsg, strlen(errmsg));
    reply = reply->append("\r\n", 2);

    return {reply, &Sds::destroy};
}

std::function<void(shared_ptr<Connection>, Command&)> GetCommandHandler(Sds* cmdtype)
{
    std::string command(cmdtype->buf, cmdtype->length());
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    return commands_map.contains(command) ? commands_map[command] : nullptr;
}

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>& result)
{
    vector<unique_ptr<ValueRef>> temp;
    temp.push_back(std::move(result));
    return GenerateReply(temp);
}

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>&& result)
{
    vector<unique_ptr<ValueRef>> temp;
    temp.push_back(std::move(result));
    return GenerateReply(temp);
}

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(std::vector<unique_ptr<ValueRef>>& result)
{
    int size = result.size();
    std::string temp = std::to_string(size);

    Sds* reply = Sds::create("*", 1, 1);
    reply = reply->append(temp.c_str(), temp.length());
    reply = reply->append("\r\n", 2);

    for (auto& vr : result)
    {
        if (vr == nullptr)
        {
            reply = reply->append("$3\r\nnil\r\n", 10);
            continue;
        }

        temp = std::to_string(vr->val->length());
        reply = reply->append("$", 1);
        reply = reply->append(temp.c_str(), temp.length());
        reply = reply->append("\r\n", 2);
        reply = reply->append(vr->val);
        reply = reply->append("\r\n", 2);
    }

    return {reply, &Sds::destroy};
}

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(std::vector<unique_ptr<ValueRef>>&& result)
{
    vector<unique_ptr<ValueRef>> temp = std::move(result);
    return GenerateReply(temp);
}