#pragma once

#include <asio.hpp>

#include <vector>
#include <iostream>
#include <exception>

#include "db.h"
#include "Command/command.h"
#include "Utility/utility.hpp"
#include "DataType/redisobj.h"
#include "Networking/connection.h"

using std::string;
using std::vector;
using std::weak_ptr;
using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

class Server;

extern Server server;
extern uint32_t DATABASE_NUM;

class Server
{
public:
	asio::io_context io_context;
	std::vector<RedisDb> databases;
	std::atomic<uint64_t> connection_id;

	//execute thread
	asio::static_thread_pool exec_threadpool;

	asio::steady_timer delobj_timer;

public:
	Server();

	void start();

	awaitable<void> delObjectHandler();
	
	awaitable<void> listener();
	awaitable<void> handleConnection(shared_ptr<Connection> conn);
	awaitable<Command> readCommandFromClient(shared_ptr<Connection> conn);

	RedisDb* selectDb(Sds* key);
	std::function<void(shared_ptr<Connection>, Command&)> CommandProcess(Command& cmd);
};
