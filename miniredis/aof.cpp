#include "aof.h"

unordered_set<string_view> cmd_need_aof = {
    "set",   "get",     "incr", "decr", "append", // string command
    "hset",  "hdel",                              // hash command
    "lpush", "rpush",   "lpop", "rpop",           // list command
    "sadd",  "srem",                              // set command
    "zadd",  "zrem",                              // zset command
    "del",   "flushall"                           // general command
};

Aof::Aof(thread_pool& work_executor, io_context& io_context) : aof_timer(io_context)
{
    co_spawn(work_executor, aofTimerHandler(), detached);
}

Aof::~Aof()
{
    aof_timer.cancel();
}

void Aof::simplifyAOF()
{
    // TODO: simplify aof
}

void Aof::loadAOF(const string& path)
{
    ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs)
        throw std::runtime_error("open aof file failed");

    // TODO: load aof
}

void Aof::storeAOF(const string& path)
{
}

void Aof::addCmdToAof(Command& cmd)
{
    aof_cmds.push_back(cmd);
}

awaitable<void> Aof::aofTimerHandler()
{
    while (true)
    {
        aof_timer.expires_after(std::chrono::seconds(AOF_TIMER_INTERVAL));
        co_await aof_timer.async_wait(use_awaitable);

        // TODO: simplify aof

        // TODO: store aof
    }
}

bool Aof::isCmdNeedAof(Sds* cmdtype)
{
    return cmd_need_aof.contains(string_view(cmdtype->buf, cmdtype->length()));
}