#pragma once
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "utility.hpp"

using namespace std;

inline void WorkerThread()
{
    try
    {
        asio::io_context io_context;

        auto socket = make_unique<asio::ip::tcp::socket>(io_context);
        asio::ip::tcp::resolver resolver(io_context);
        asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));

        vector<string> testKeys;
        static thread_local mt19937 rng{random_device{}()};
        static atomic<bool> running{true};

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&](const asio::error_code& error, int signal_number)
            {
                running = false;
                // Cleanup keys
                for (const auto& key : testKeys)
                {
                    vector<string> delArgs = {"DEL", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
                }

                socket->shutdown(asio::ip::tcp::socket::shutdown_both);
                socket->close();
            });

        uniform_int_distribution<> typeDist(0, 4);
        uniform_int_distribution<> cmdDist(0, 4);

        while (running)
        {
            int type = typeDist(rng);
            string key = GetRandomString(10);

            switch (type)
            {
            case 0: // String commands
            {
                int cmd = cmdDist(rng);
                switch (cmd)
                {
                case 0: // SET
                {
                    string value = GetRandomString(20);
                    vector<string> args = {"SET", key, value};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 1: // GET
                {
                    vector<string> args = {"GET", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 2: // INCR
                {
                    vector<string> args = {"INCR", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 3: // DECR
                {
                    vector<string> args = {"DECR", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 4: // APPEND
                {
                    string value = GetRandomString(10);
                    vector<string> args = {"APPEND", key, value};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                }
                testKeys.push_back(key);
                break;
            }
            case 1: // Hash commands
            {
                int cmd = cmdDist(rng);
                string field = GetRandomString(8);
                switch (cmd)
                {
                case 0: // HSET
                {
                    string value = GetRandomString(20);
                    vector<string> args = {"HSET", key, field, value};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 1: // HGET
                {
                    vector<string> args = {"HGET", key, field};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 2: // HDEL
                {
                    vector<string> args = {"HDEL", key, field};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 3: // HEXISTS
                {
                    vector<string> args = {"HEXISTS", key, field};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 4: // HGETALL
                {
                    vector<string> args = {"HGETALL", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                }
                testKeys.push_back(key);
                break;
            }
            case 2: // List commands
            {
                int cmd = cmdDist(rng);
                switch (cmd)
                {
                case 0: // LPUSH
                {
                    string value = GetRandomString(20);
                    vector<string> args = {"LPUSH", key, value};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 1: // RPUSH
                {
                    string value = GetRandomString(20);
                    vector<string> args = {"RPUSH", key, value};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 2: // LPOP
                {
                    vector<string> args = {"LPOP", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 3: // RPOP
                {
                    vector<string> args = {"RPOP", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 4: // LLEN
                {
                    vector<string> args = {"LLEN", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                }
                testKeys.push_back(key);
                break;
            }
            case 3: // Set commands
            {
                int cmd = cmdDist(rng);
                string member = GetRandomString(20);
                switch (cmd)
                {
                case 0: // SADD
                {
                    vector<string> args = {"SADD", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 1: // SREM
                {
                    vector<string> args = {"SREM", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 2: // SISMEMBER
                {
                    vector<string> args = {"SISMEMBER", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 3: // SMEMBERS
                {
                    vector<string> args = {"SMEMBERS", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 4: // SCARD
                {
                    vector<string> args = {"SCARD", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                }
                testKeys.push_back(key);
                break;
            }
            case 4: // ZSet commands
            {
                int cmd = cmdDist(rng);
                string member = GetRandomString(20);
                double score = uniform_real_distribution<>(0, 100)(rng);
                switch (cmd)
                {
                case 0: // ZADD
                {
                    vector<string> args = {"ZADD", key, to_string(score), member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 1: // ZREM
                {
                    vector<string> args = {"ZREM", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 2: // ZSCORE
                {
                    vector<string> args = {"ZSCORE", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 3: // ZRANK
                {
                    vector<string> args = {"ZRANK", key, member};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                case 4: // ZCARD
                {
                    vector<string> args = {"ZCARD", key};
                    asio::write(*socket, asio::buffer(ConvertToResp(args)));
                    break;
                }
                }
                testKeys.push_back(key);
                break;
            }
            }
        }
    }
    catch (exception& e)
    {
        cerr << "Worker Exception: " << e.what() << endl;
    }
}

inline void TestMultiThreadRandomCommands()
{
    cout << "Starting multi-threaded random commands test..." << endl;

    asio::io_context io_context;
    vector<thread> threads;
    const int num_threads = thread::hardware_concurrency() * 6;

    // Create worker threads
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&io_context]() { WorkerThread(); });
    }

    // Run for a fixed duration
    this_thread::sleep_for(chrono::seconds(30));

    // Wait for all threads to complete
    for (auto& t : threads)
    {
        t.join();
    }

    cout << "Multi-threaded random commands test completed" << endl;
}