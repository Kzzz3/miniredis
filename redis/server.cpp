module;

#include <asio.hpp>

module server;

using std::string;
using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::streambuf;
using asio::awaitable;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

void server::start()
{
    co_spawn(io_context, listener(), detached);
	asio::signal_set signals(io_context, SIGINT, SIGTERM);
	signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });

    io_context.run();
}


awaitable<void> server::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { tcp::v4(), 10087 });
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);

		Connection conn(io_context, connection_id);
		conn.db = &databases[0];
        co_spawn(executor, handleConnection(std::move(conn)), detached);
    }
}

awaitable<void> server::handleConnection(Connection conn) {
    conn.state = ConnectionState::CONN_STATE_CONNECTED;

    for (;;) {
        //read mutibulk len
		Command cmd = co_await readCommandFromClient(conn);

		//execute command
		//std::function<void(Connection& conn, Command&)> handler = GetCommandHandler(cmd[0]);
		//if (handler) {
		//	handler(conn ,cmd);
		//}
		//else {
		//	//err
		//}

	}
}

awaitable<Command> server::readCommandFromClient(Connection& conn)
{
	size_t n = co_await async_read_until(conn.socket, *conn.buffer, "\r\n", use_awaitable);
	if (n == 0) {
        // close command
	}

	const char* data = asio::buffer_cast<const char*>(conn.buffer->data());
	if (data[0] != '*') {
		//err
	}
	
	size_t bulkSize = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
	if (bulkSize <= 0) {
		//err
	}
	conn.buffer->consume(n);

	Command cmd;
	cmd.reserve(bulkSize);

	size_t bulkLen = 0;
	for (size_t i = 0; i < bulkSize; i++) {
		n = co_await async_read_until(conn.socket, *conn.buffer, "\r\n", use_awaitable);
		if (n == 0) {
			// close command
		}

		data = asio::buffer_cast<const char*>(conn.buffer->data());
		switch (data[0])
		{
		case '+':
			cmd[i] = sds::create(data + 1, n - 3, n - 3);
			conn.buffer->consume(n);
			break;
		case ':':
			cmd[i] = sds::create(data + 1, n - 3, n - 3);
			conn.buffer->consume(n);
			break;
		case '$':
			bulkLen = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
			if (bulkLen <= 0) {
				//err
			}
			conn.buffer->consume(n);
			n = co_await async_read(conn.socket, *conn.buffer, asio::transfer_exactly(bulkLen + 2), use_awaitable);
			if (n == 0) {
				// close command
			}

			data = asio::buffer_cast<const char*>(conn.buffer->data());
			cmd[i] = sds::create(data, bulkLen, bulkLen);
			conn.buffer->consume(n);
			break;
		default:
			break;
		}
	}
	co_return cmd;
}