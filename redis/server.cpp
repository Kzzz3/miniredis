#include "server.h"

Server server;
uint32_t DATABASE_NUM = 16;

Server::Server() :
	connection_id(0),
	databases(DATABASE_NUM)
{
	write_thread = std::thread([this]() {
		for (;;) {
			Sleep(1);
			if (!response_queue.empty()) {
				Response resp = std::move(response_queue.front());
				response_queue.pop_front();
				auto conn = resp.conn.lock();
				if (conn) {
					conn->AsyncSend(resp.reply.get());
				}
			}
		}
	});
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
		shared_ptr<Connection> conn = std::make_shared<Connection>(connection_id++, std::move(socket));

        co_spawn(executor, handleConnection(conn), detached);
    }
}

awaitable<void> Server::handleConnection(shared_ptr<Connection> conn) {
	conn->state = ConnectionState::CONN_STATE_CONNECTED;

	for (;;) {
		//read mutibulk len
		Command cmd = co_await readCommandFromClient(conn);

		//command process
		std::function<void(shared_ptr<Connection> conn, Command&)> handler = CommandProcess(cmd);

		//execute command
		if (handler) {
			handler(conn ,cmd);
		}
		else {
			//err
		}

	}
}

awaitable<Command> Server::readCommandFromClient(shared_ptr<Connection> conn)
{
	size_t n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);
	if (n == 0) {
        // close command
	}

	const char* data = asio::buffer_cast<const char*>(conn->read_buffer->data());
	if (data[0] != '*') {
		//err
	}
	
	size_t bulkSize = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
	if (bulkSize <= 0) {
		//err
	}
	conn->read_buffer->consume(n);

	Command cmd;
	cmd.reserve(bulkSize);

	size_t bulkLen = 0;
	for (size_t i = 0; i < bulkSize; i++) {
		n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);
		if (n == 0) {
			// close command
		}

		data = asio::buffer_cast<const char*>(conn->read_buffer->data());
		switch (data[0])
		{
		case '+':
			cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
			conn->read_buffer->consume(n);
			break;
		case ':':
			cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
			conn->read_buffer->consume(n);
			break;
		case '$':
			bulkLen = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
			if (bulkLen <= 0) {
				//err
			}
			conn->read_buffer->consume(n);

			if (conn->read_buffer->in_avail() < bulkLen + 2) {
				n = co_await async_read(conn->socket, *conn->read_buffer, asio::transfer_exactly(bulkLen + 2 - conn->read_buffer->in_avail()), use_awaitable);
				if (n == 0) {
					// close command
				}
			}

			data = asio::buffer_cast<const char*>(conn->read_buffer->data());
			cmd.emplace_back(Sds::create(data, bulkLen, bulkLen));
			conn->read_buffer->consume(bulkLen + 2);
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

std::function<void(shared_ptr<Connection>, Command&)> Server::CommandProcess(Command& cmd)
{
	return GetCommandHandler(cmd[0]);
}
