#pragma once

#include <asio.hpp>

#include <thread>
#include <vector>
#include <iostream>
#include <exception>
#include <filesystem>

#include "Database/db.h"
#include "Command/command.h"
#include "Utility/utility.hpp"
#include "DataType/redisobj.h"
#include "Networking/connection.h"

using std::thread;
using std::string;
using std::vector;
using std::atomic;
using std::weak_ptr;
using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::signal_set;
using asio::thread_pool;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

extern bool AOF_ENABLED;
extern bool RDB_ENABLED;
extern size_t DATABASE_NUM;
extern size_t RDB_TIMER_INTERVAL;
extern size_t AOF_TIMER_INTERVAL;
extern size_t DEL_TIMER_INTERVAL;

constexpr size_t IO_THREAD_NUM = 2;
constexpr size_t EXEC_THREAD_NUM = 1;

class Server
{
public:
    asio::io_context io_context; // io_context
    thread_pool exec_threadpool; // execute thread

    RedisDb database; // database and RDB

    signal_set signals;             // signal set
    atomic<uint64_t> connection_id; // connection id
public:
    Server();
    ~Server();

    awaitable<void> listenerHandler();
    awaitable<void> handleConnection(shared_ptr<Connection> conn);
    awaitable<Command> readCommandFromClient(shared_ptr<Connection> conn);
    std::function<bool(shared_ptr<Connection>, Command&)> CommandProcess(Command& cmd);
};
extern Server server;