#include "server.h"

Server server;
uint32_t DATABASE_NUM = 16;

Server::Server()
    : connection_id(0), databases(DATABASE_NUM), io_context(16), exec_threadpool(1),
      delobj_timer(io_context)
{
    if (std::filesystem::exists("rdb.dat.gz"))
        DecompressFileStream("rdb.dat.gz", "rdb.dat");

    if (std::filesystem::exists("rdb.dat"))
        loadRDB("rdb.dat");
}

void Server::start()
{
    co_spawn(io_context, listenerHandler(), detached);
    co_spawn(exec_threadpool, delObjectHandler(), detached);

    // listen for signals
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const asio::error_code&, int) { io_context.stop(); });

    io_context.run();
}

void Server::loadRDB(const string& path)
{
    ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs)
        throw std::runtime_error("open rdb file failed");

    for (auto& db : databases)
    {
        db.deserialize_from(ifs);
    }
}

void Server::storeRDB(const string& path)
{
    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    ofstream ofs(path, std::ios::out | std::ios::binary);
    if (!ofs)
        throw std::runtime_error("open rdb file failed");

    for (auto& db : databases)
    {
        db.serialize_to(ofs);
    }
}

awaitable<void> Server::delObjectHandler()
{
    for (;;)
    {
        delobj_timer.expires_after(std::chrono::seconds(60));
        co_await delobj_timer.async_wait(use_awaitable);
        for (auto& db : databases)
        {
            std::list<RedisObj*>& deadobj = *db.deadobj;
            for (auto& obj : deadobj)
            {
                if (obj->refcount == 0)
                {
                    delete obj;
                }
            }
        }

        storeRDB("rdb_temp.dat");
        CompressFileStream("rdb_temp.dat", "rdb.dat.gz");
    }
}

awaitable<void> Server::listenerHandler()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 10087});
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        shared_ptr<Connection> conn =
            std::make_shared<Connection>(connection_id++, std::move(socket));

        co_spawn(executor, handleConnection(conn), detached);
    }
}

awaitable<void> Server::handleConnection(shared_ptr<Connection> conn)
{
    conn->state = ConnectionState::CONN_STATE_CONNECTED;

    for (;;)
    {
        // read mutibulk len
        Command cmd = co_await readCommandFromClient(conn);
        if (cmd.empty())
        {
            conn->Close();
            co_return;
        }

        // command process
        std::function<void(shared_ptr<Connection> conn, Command&)> handler = CommandProcess(cmd);

        // execute command
        if (handler)
        {
            asio::post(exec_threadpool,
                       [conn, handler, cmd]() mutable
                       {
                           handler(conn, cmd);
                           for (auto& sds : cmd)
                               Sds::destroy(sds);
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

RedisDb* Server::selectDb(Sds* key)
{
    return &databases[std::hash<Sds*>{}(key) % DATABASE_NUM];
}

std::function<void(shared_ptr<Connection>, Command&)> Server::CommandProcess(Command& cmd)
{
    // check command
    if (cmd.size() == 0)
        return nullptr;

    return GetCommandHandler(cmd[0]);
}