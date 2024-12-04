#pragma once

#include <asio.hpp>

#include <vector>
#include <iostream>
#include <exception>

#include "db.h"
#include "command.h"
#include "utility.hpp"
#include "DataType/redisobj.h"
#include "Networking/connection.h"

using std::string;
using std::vector;
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

public:
	Server();

	void start();
	
	awaitable<void> listener();
	awaitable<void> handleConnection(Connection conn);
	awaitable<Command> readCommandFromClient(Connection& conn);

	RedisDb* selectDb(Sds* key);
};
