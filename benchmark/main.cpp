#include <atomic>
#include <thread>
#include <iostream>
#include <asio.hpp>

#include "RandomCommand.hpp"

using std::cout;
using std::endl;
using std::thread;
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

    int thread_num = thread::hardware_concurrency();
    cout << "thread_num: " << thread_num << endl;

    thread_pool pool(thread_num);
    for (int i = 0; i < thread_num; ++i)
    {
        asio::post(pool, [&]() { RandomCommand(ctxIo, stop); });
    }

    pool.join();
    ioThread.join();
    return 0;
}