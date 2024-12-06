#pragma once

#include <asio.hpp>

#include <vector>
#include <iostream>
#include <exception>

#include "db.h"
#include "utility.hpp"
#include "Command/command.h"
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

class Response
{
public:
	unique_ptr<Sds, decltype(&Sds::destroy)> reply;
	std::weak_ptr<Connection> conn;

	Response(unique_ptr<Sds, decltype(&Sds::destroy)>&& reply, std::weak_ptr<Connection> conn) : reply(std::move(reply)), conn(conn) {}
};

class Server
{
public:
	asio::io_context io_context;
	std::vector<RedisDb> databases;
	std::atomic<uint64_t> connection_id;

	//reade and parse command

	//execute thread
	std::list<Command> execute_queue;
	std::thread execute_thread;

	//write thread
	std::list<Response> response_queue;
	std::thread write_thread;

	//to do: use asio::strand to write response to client instead of using a single thread

public:
	Server();

	void start();
	
	awaitable<void> listener();
	awaitable<void> handleConnection(shared_ptr<Connection> conn);
	awaitable<Command> readCommandFromClient(shared_ptr<Connection> conn);

	RedisDb* selectDb(Sds* key);
	std::function<void(shared_ptr<Connection>, Command&)> CommandProcess(Command& cmd);
};
