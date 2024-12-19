#pragma once
#include <asio.hpp>

#include <list>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

#include "DataStruct/sds.h"
#include "Command/command.h"

using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::io_context;
using asio::thread_pool;
using asio::steady_timer;
using asio::use_awaitable;

using std::list;
using std::unordered_set;
using std::unordered_map;

constexpr uint64_t AOF_TIMER_INTERVAL = 10;

class Aof
{
public:
    Aof(thread_pool& work_executor, io_context& io_context);
    ~Aof();

    void simplifyAOF();
    void addCmdToAof(Command& cmd);
    void loadAOF(const string& filename);
    void storeAOF(const string& filename);

    awaitable<void> aofTimerHandler();

    static bool isCmdNeedAof(Sds* cmdtype);
public:
    steady_timer aof_timer;
    list<Command> aof_cmds;
};
