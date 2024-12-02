module;

#include <asio.hpp>

export module server;

import db;
import sds;
import dict;
import command;
import connection;

using std::vector;
using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

export class server
{
public:
	asio::io_context io_context;
	std::atomic<uint64_t> connection_id = 0;

	RedisDb databases[16];
	

public:
	void start();
	
	awaitable<void> listener();
	awaitable<void> handleConnection(Connection conn);
	awaitable<Command> readCommandFromClient(Connection& conn);
};