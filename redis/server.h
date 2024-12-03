#pragma once

#include <asio.hpp>

#include <vector>
#include <iostream>
#include <exception>

#include "utility.h"
#include "command.h"
#include "DataType/redisobj.h"
#include "Networking/connection.h"

using std::vector;
using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

class server
{
public:
	asio::io_context io_context;
	std::atomic<uint64_t> connection_id = 0;

	RedisDb databases;

public:
	void start();
	
	awaitable<void> listener();
	awaitable<void> handleConnection(Connection conn);
	awaitable<Command> readCommandFromClient(Connection& conn);
};