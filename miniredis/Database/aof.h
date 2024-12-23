#pragma once
#include <asio.hpp>
#include <ylt/struct_pack.hpp>

#include <list>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

#include "DataStruct/sds.h"

using std::list;
using std::pair;
using std::unordered_set;
using std::unordered_map;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::steady_timer;
using asio::use_awaitable;

using Command = std::vector<Sds*>;

extern bool AOF_ENABLED;
extern size_t AOF_TIMER_INTERVAL;

class Aof
{
public:
    Aof(asio::thread_pool& work_executor, asio::io_context& io_context);
    ~Aof();

    // handler
    awaitable<void> aofTimerHandler();

    // AOF operation
    void startAof();
    void simplifyAOF();
    void addCmdToAof(Command& cmd);
    void loadAOF(const string& filename);
    void storeAOF(const string& filename);

    // simplify command
    bool simplifyStringCommand(int index, Command& cmd);
    bool simplifyListCommand(int index, Command& cmd);
    bool simplifyHashCommand(int index, Command& cmd);
    bool simplifySetCommand(int index, Command& cmd);
    bool simplifyZSetCommand(int index, Command& cmd);
    bool simplifyGeneralCommand(int index, Command& cmd);

    static bool isCmdNeedAof(Sds* cmdtype);

public:
    asio::io_context& io_context;
    asio::thread_pool& work_executor;

    steady_timer aof_timer;
    vector<Command> aof_cmds;

    // temp container used in simplifyAOF
    vector<Sds*> temp_del_values;
    list<pair<int, Command&>> temp_general_cmds;
    unordered_map<Sds*, list<pair<int, Command&>>> temp_cmds;
};