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

inline void TestListCommands()
{
    cout << "Starting list commands test..." << endl;
    asio::io_context io_context;
    auto socket = make_unique<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));

    unordered_map<string, vector<string>> testKeys;
    static thread_local mt19937 rng{random_device{}()};

    for (int i = 0; i < 10000; i++)
    {
        string key = GetRandomString(10);
        string value = GetRandomString(20);

        // LPUSH command
        vector<string> lpushArgs = {"LPUSH", key, value};
        asio::write(*socket, asio::buffer(ConvertToResp(lpushArgs)));

        // RPUSH command
        vector<string> rpushArgs = {"RPUSH", key, value};
        asio::write(*socket, asio::buffer(ConvertToResp(rpushArgs)));

        // LPOP/RPOP command - randomly get existing or non-existing key
        uniform_int_distribution<> dist(0, 100);
        if (dist(rng) < 80 && !testKeys.empty()) // 80% chance to get existing key
        {
            auto it = testKeys.begin();
            advance(it, uniform_int_distribution<>(0, testKeys.size() - 1)(rng));
            vector<string> popArgs = {(dist(rng) < 50 ? "LPOP" : "RPOP"), it->first};
            asio::write(*socket, asio::buffer(ConvertToResp(popArgs)));
        }
        else // 20% chance to get random key
        {
            string randomKey = GetRandomString(10);
            vector<string> popArgs = {(dist(rng) < 50 ? "LPOP" : "RPOP"), randomKey};
            asio::write(*socket, asio::buffer(ConvertToResp(popArgs)));
        }

        testKeys[key].push_back(value);
    }

    // Cleanup all list keys
    for (const auto& pair : testKeys)
    {
        vector<string> delArgs = {"DEL", pair.first};
        asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
    }

    socket->shutdown(asio::ip::tcp::socket::shutdown_both);
    socket->close();
    cout << "List commands test completed" << endl;
}