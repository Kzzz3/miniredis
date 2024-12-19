#include "server.h"

Server server;

Server::Server()
    : io_context(16), exec_threadpool(1), aof(exec_threadpool, io_context),
      database(exec_threadpool, io_context), signals(io_context, SIGINT, SIGTERM), connection_id(0)
{
    if (std::filesystem::exists("rdb.dat.gz"))
    {
        std::cout << "decompress rdb.dat.gz..." << std::endl;
        DecompressFileStream("rdb.dat.gz", "rdb.dat");
        std::cout << "decompress rdb.dat.gz done" << std::endl;
    }

    if (std::filesystem::exists("rdb.dat"))
    {
        std::cout << "loading rdb..." << std::endl;
        database.loadRDB("rdb.dat");
        std::cout << "loading rdb done" << std::endl;
    }

    // listen for signals
    signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });

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
        // read mutibulk len
        Command cmd = co_await readCommandFromClient(conn);
        if (cmd.empty())
        {
            conn->Close();
            co_return;
        }

        // command process
        std::function<bool(shared_ptr<Connection> conn, Command&)> handler = CommandProcess(cmd);

        // execute command
        if (handler)
        {
            asio::post(exec_threadpool,
                       [this, conn, handler, cmd]() mutable
                       {
                           bool success = handler(conn, cmd);
                           if (AOF_ENABLED && success && Aof::isCmdNeedAof(cmd[0]))
                           {
                               aof.addCmdToAof(cmd);
                           }
                           else
                           {
                               for (auto& sds : cmd)
                                   Sds::destroy(sds);
                           }

                           std::cout << Allocator::current_allocated << std::endl;
                       });
        }
        else
        {
            // err
            for (auto& sds : cmd)
                Sds::destroy(sds);
        }
    }
}

awaitable<Command> Server::readCommandFromClient(shared_ptr<Connection> conn)
{

    size_t n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);
    const char* data = asio::buffer_cast<const char*>(conn->read_buffer->data());
    if (data[0] != '*')
    {
        conn->read_buffer->consume(n);
        throw asio::system_error(asio::error::invalid_argument, "invalid command");
    }

    size_t bulkSize = std::stoi(std::string(data + 1, n - 3), nullptr, 10);
    if (bulkSize <= 0)
    {
        // err
        conn->read_buffer->consume(n);
        throw asio::system_error(asio::error::invalid_argument, "invalid command");
    }
    conn->read_buffer->consume(n);

    Command cmd;
    cmd.reserve(bulkSize);

    try
    {
        int64_t bulkLen = 0;
        for (size_t i = 0; i < bulkSize; i++)
        {
            n = co_await async_read_until(conn->socket, *conn->read_buffer, "\r\n", use_awaitable);

            optional<int64_t> num;
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
                num = str2num<int64_t>(data + 1, n - 3);
                if (!num.has_value() || num.value() <= 0)
                {
                    throw asio::system_error(asio::error::invalid_argument, "invalid command");
                }

                bulkLen = num.value();
                conn->read_buffer->consume(n);
                if (conn->read_buffer->in_avail() < bulkLen + 2)
                {
                    n = co_await async_read(
                        conn->socket, *conn->read_buffer,
                        asio::transfer_exactly(bulkLen + 2 - conn->read_buffer->in_avail()),
                        use_awaitable);
                }

                data = asio::buffer_cast<const char*>(conn->read_buffer->data());
                cmd.emplace_back(Sds::create(data, bulkLen, bulkLen));
                conn->read_buffer->consume(bulkLen + 2);
                break;
            default:
                throw asio::system_error(asio::error::invalid_argument, "invalid command");
            }
        }
        co_return cmd;
    }
    catch (...)
    {
        for (auto& sds : cmd)
            Sds::destroy(sds);
        co_return Command();
    }
}

std::function<bool(shared_ptr<Connection>, Command&)> Server::CommandProcess(Command& cmd)
{
    // check command
    if (cmd.size() == 0)
        return nullptr;

    return GetCommandHandler(cmd[0]);
}