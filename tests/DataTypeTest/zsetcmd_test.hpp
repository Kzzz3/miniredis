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

inline void TestZSetCommands(int num)
{
    cout << "Starting sorted set commands test..." << endl;
    asio::io_context io_context;
    auto socket = make_unique<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));

    unordered_map<string, vector<string>> testKeys;
    static thread_local mt19937 rng{random_device{}()};

    for (int i = 0; i < num; i++)
    {
        string key = GetRandomString(10);
        string member = GetRandomString(20);
        string score = to_string(uniform_int_distribution<>(0, 999)(rng));

        // ZADD command
        vector<string> zaddArgs = {"ZADD", key, score, member};
        asio::write(*socket, asio::buffer(ConvertToResp(zaddArgs)));

        // ZRANGE/ZREVRANGE command - randomly get existing or non-existing key
        uniform_int_distribution<> dist(0, 100);
        if (dist(rng) < 80 && !testKeys.empty()) // 80% chance to get existing key
        {
            auto it = testKeys.begin();
            advance(it, uniform_int_distribution<>(0, testKeys.size() - 1)(rng));
            if (dist(rng) < 50)
            {
                vector<string> zrangeArgs = {"ZRANGE", it->first, "0", "-1"};
                asio::write(*socket, asio::buffer(ConvertToResp(zrangeArgs)));
            }
            else
            {
                vector<string> zrevrangeArgs = {"ZREVRANGE", it->first, "0", "-1"};
                asio::write(*socket, asio::buffer(ConvertToResp(zrevrangeArgs)));
            }
        }
        else // 20% chance to get random key
        {
            string randomKey = GetRandomString(10);
            if (dist(rng) < 50)
            {
                vector<string> zrangeArgs = {"ZRANGE", randomKey, "0", "-1"};
                asio::write(*socket, asio::buffer(ConvertToResp(zrangeArgs)));
            }
            else
            {
                vector<string> zrevrangeArgs = {"ZREVRANGE", randomKey, "0", "-1"};
                asio::write(*socket, asio::buffer(ConvertToResp(zrevrangeArgs)));
            }
        }

        testKeys[key].push_back(score);
        testKeys[key].push_back(member);
    }

    // Cleanup all sorted set keys
    for (const auto& pair : testKeys)
    {
        vector<string> delArgs = {"DEL", pair.first};
        asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
    }
    cout << "Sorted set commands test completed" << endl;
}