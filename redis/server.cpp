#include "server.h"

Server server;
uint32_t DATABASE_NUM = 16;

Server::Server() :
	connection_id(0),
	databases(DATABASE_NUM)
{
}

void Server::start()
{
    co_spawn(io_context, listener(), detached);
	asio::signal_set signals(io_context, SIGINT, SIGTERM);
	signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });

	io_context.run();
}


awaitable<void> Server::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { tcp::v4(), 10087 });
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);

		Connection conn(connection_id++, std::move(socket));
        co_spawn(executor, handleConnection(std::move(conn)), detached);
    }
}

awaitable<void> Server::handleConnection(Connection conn) {
	conn.state = ConnectionState::CONN_STATE_CONNECTED;

	for (;;) {
		//read mutibulk len
		Command cmd = co_await readCommandFromClient(conn);

		//command process

		//execute command
		std::function<void(Connection& conn, Command&)> handler = GetCommandHandler(cmd[0]);
		if (handler) {
			handler(conn ,cmd);
		}
		else {
			//err
		}

	}
}

awaitable<Command> Server::readCommandFromClient(Connection& conn)
{
	size_t n = co_await async_read_until(conn.socket, *conn.read_buffer, "\r\n", use_awaitable);
	if (n == 0) {
        // close command
	}

	const char* data = asio::buffer_cast<const char*>(conn.read_buffer->data());
	if (data[0] != '*') {
		//err
	}
	
	size_t bulkSize = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
	if (bulkSize <= 0) {
		//err
	}
	conn.read_buffer->consume(n);

	Command cmd;
	cmd.reserve(bulkSize);

	size_t bulkLen = 0;
	for (size_t i = 0; i < bulkSize; i++) {
		n = co_await async_read_until(conn.socket, *conn.read_buffer, "\r\n", use_awaitable);
		if (n == 0) {
			// close command
		}

		data = asio::buffer_cast<const char*>(conn.read_buffer->data());
		switch (data[0])
		{
		case '+':
			cmd[i] = Sds::create(data + 1, n - 3, n - 3);
			conn.read_buffer->consume(n);
			break;
		case ':':
			cmd[i] = Sds::create(data + 1, n - 3, n - 3);
			conn.read_buffer->consume(n);
			break;
		case '$':
			bulkLen = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
			if (bulkLen <= 0) {
				//err
			}
			conn.read_buffer->consume(n);

			if (conn.read_buffer->in_avail() < bulkLen + 2) {
				n = co_await async_read(conn.socket, *conn.read_buffer, asio::transfer_exactly(bulkLen + 2 - conn.read_buffer->in_avail()), use_awaitable);
				if (n == 0) {
					// close command
				}
			}

			data = asio::buffer_cast<const char*>(conn.read_buffer->data());
			cmd.push_back(Sds::create(data, bulkLen, bulkLen));
			conn.read_buffer->consume(bulkLen + 2);
			break;
		default:
			break;
		}
	}
	co_return cmd;
}

RedisDb* Server::selectDb(Sds* key)
{
	return &databases[std::hash<Sds*>{}(key) % DATABASE_NUM];
}
