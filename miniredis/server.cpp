#include "server.h"

bool RDB_ENABLED = false;
bool AOF_ENABLED = false;
size_t DATABASE_NUM = 16;
size_t RDB_TIMER_INTERVAL = 60;
size_t AOF_TIMER_INTERVAL = 60;
size_t DEL_TIMER_INTERVAL = 60;

Server server;

Server::Server()
    : io_context(8), exec_threadpool(1), database(exec_threadpool, io_context),
      signals(io_context, SIGINT, SIGTERM), connection_id(0),
      processed_print_timer(io_context, std::chrono::seconds(1)), total_commands_received(0),
      total_commands_processed(0)
{
    // listen for signals
    signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });

    // print total commands processed every second
    static auto printProcessed = [this]() -> awaitable<void>
    {
        while (true)
        {
            processed_print_timer.expires_after(std::chrono::seconds(1));
            co_await processed_print_timer.async_wait(use_awaitable);
            // printf("total commands received: %lu\n", total_commands_received.load());
            // printf("total commands processed: %lu\n", total_commands_processed.load());
            // printf("current allocated: %lu\n", Allocator::current_allocated.load());
            total_commands_received = 0;
            total_commands_processed = 0;
        }
    };
    co_spawn(io_context, printProcessed(), detached);

    // listen for clients
    co_spawn(io_context, listenerHandler(), detached);

    for (size_t i = 0; i < IO_THREAD_NUM; i++)
    {
        thread([&]() { io_context.run(); }).detach();
    }
}

Server::~Server()
{
}

awaitable<void> Server::listenerHandler()
{
    tcp::acceptor acceptor(io_context, {tcp::v4(), 10087});
    while (true)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        shared_ptr<Connection> conn =
            std::make_shared<Connection>(connection_id++, std::move(socket));

        co_spawn(io_context, handleConnection(conn), detached);
    }
}

awaitable<void> Server::handleConnection(shared_ptr<Connection> conn)
{
    conn->state = ConnectionState::CONN_STATE_CONNECTED;

    while (true)
    {
        // read command
        auto result = co_await readCommandFromClient(conn);
        if (!result)
        {
            conn->Close();
            co_return;
        }

        Command cmd = result.value();
        cmd[0]->convertToLower();

        // command process
        std::function<bool(shared_ptr<Connection> conn, Command&)> handler = CommandProcess(cmd);

        // execute command
        if (handler)
        {
            total_commands_received++;
            asio::post(exec_threadpool,
                       [this, conn, handler, cmd]() mutable
                       {
                           bool success = handler(conn, cmd);
                           if (AOF_ENABLED && success && Aof::isCmdNeedAof(cmd[0]))
                           {
                               database.aof.addCmdToAof(cmd);
                           }
                           else
                           {
                               for (auto& sds : cmd)
                                   Sds::destroy(sds);
                           }
                           total_commands_processed++;
                           //    std::cout << Allocator::current_allocated << std::endl;
                       });
        }
        else
        {
            for (auto& sds : cmd)
                Sds::destroy(sds);
        }
    }
}

awaitable<std::expected<Command, std::error_code>> Server::readCommandFromClient(
    shared_ptr<Connection> conn)
{
    size_t n = 0;
    const char* data = nullptr;

    n = co_await async_read_until(conn->socket, conn->read_buffer, "\r\n", use_awaitable);
    data = asio::buffer_cast<const char*>(conn->read_buffer.data());

    // Handle PING
    if (strncmp((char*)data, "PING\r\n", n) == 0)
    {
        conn->read_buffer.consume(n);
        co_await asio::async_write(conn->socket, asio::buffer("+PONG\r\n", 7), use_awaitable);
        co_return co_await readCommandFromClient(conn);
    }

    // Check for array prefix
    if (data[0] != '*')
    {
        conn->read_buffer.consume(n);
        co_return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    // Parse array size
    auto num = str2num<size_t>(data + 1, n - 3);
    if (!num.has_value() || num.value() <= 0)
    {
        conn->read_buffer.consume(n);
        co_return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }
    size_t bulkSize = num.value();
    conn->read_buffer.consume(n);

    Command cmd;
    cmd.reserve(bulkSize);
    for (size_t i = 0; i < bulkSize; i++)
    {
        n = co_await async_read_until(conn->socket, conn->read_buffer, "\r\n", use_awaitable);
        data = asio::buffer_cast<const char*>(conn->read_buffer.data());

        int64_t bulkLen = 0;
        optional<int64_t> num;
        switch (data[0])
        {
        case '+':
            cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
            conn->read_buffer.consume(n);
            break;

        case ':':
            cmd.emplace_back(Sds::create(data + 1, n - 3, n - 3));
            conn->read_buffer.consume(n);
            break;

        case '$':
            num = str2num<int64_t>(data + 1, n - 3);
            if (!num.has_value() || num.value() <= 0)
            {
                conn->read_buffer.consume(n);
                co_return std::unexpected(std::make_error_code(std::errc::invalid_argument));
            }

            bulkLen = num.value();
            conn->read_buffer.consume(n);
            while (conn->read_buffer.in_avail() < bulkLen + 2)
            {
                n = co_await async_read(
                    conn->socket, conn->read_buffer,
                    asio::transfer_exactly(bulkLen + 2 - conn->read_buffer.in_avail()),
                    use_awaitable);
            }

            data = asio::buffer_cast<const char*>(conn->read_buffer.data());
            cmd.emplace_back(Sds::create(data, bulkLen, bulkLen));
            conn->read_buffer.consume(bulkLen + 2);
            break;

        default:
            conn->read_buffer.consume(n);
            co_return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        }
    }
    co_return cmd;
}

std::function<bool(shared_ptr<Connection>, Command&)> Server::CommandProcess(Command& cmd)
{
    // check command
    if (cmd.size() == 0)
        return nullptr;

    return GetCommandHandler(cmd[0]);
}