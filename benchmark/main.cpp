#include <atomic>
#include <thread>

#include <asio.hpp>

#include "RandomCommand.hpp"

using asio::io_context;
using asio::thread_pool;

int main()
{
    io_context ctxIo;
    atomic<bool> stop(false);
    asio::signal_set signals(ctxIo, SIGINT, SIGTERM);
    signals.async_wait(
        [&](const asio::error_code&, int)
        {
            stop = true;
            ctxIo.stop();
        });
    thread ioThread([&]() { ctxIo.run(); });

    thread_pool pool(8);
    for (int i = 0; i < 8; ++i)
    {
        asio::post(pool, [&]() { RandomCommand(ctxIo, stop); });
    }

    pool.join();
    ioThread.join();
    return 0;
}