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

// Test string type commands
inline void TestStringCommands()
{
    cout << "Starting string commands test..." << endl;

    asio::io_context io_context;
    auto socket = make_unique<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));

    vector<string> testKeys;
    static thread_local mt19937 rng{random_device{}()};

    // SET and GET test
    for (int i = 0; i < 5000; i++)
    {
        string key = GetRandomString(10);
        string value = GetRandomString(20);

        // SET command
        vector<string> setArgs = {"SET", key, value};
        asio::write(*socket, asio::buffer(ConvertToResp(setArgs)));

        // GET command - randomly get existing or non-existing key
        uniform_int_distribution<> dist(0, 100);
        if (dist(rng) < 80 && !testKeys.empty()) // 80% chance to get existing key
        {
            uniform_int_distribution<> keyDist(0, testKeys.size() - 1);
            string getKey = testKeys[keyDist(rng)];
            vector<string> getArgs = {"GET", getKey};
            asio::write(*socket, asio::buffer(ConvertToResp(getArgs)));
        }
        else // 20% chance to get random key
        {
            string randomKey = GetRandomString(10);
            vector<string> getArgs = {"GET", randomKey};
            asio::write(*socket, asio::buffer(ConvertToResp(getArgs)));
        }

        testKeys.push_back(key);
    }

    // Cleanup all string keys
    for (const auto& key : testKeys)
    {
        vector<string> delArgs = {"DEL", key};
        asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
    }
    socket->shutdown(asio::ip::tcp::socket::shutdown_both);
    socket->close();
    cout << "String commands test completed" << endl;
}