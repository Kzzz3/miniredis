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

inline void TestSetCommands(int num)
{
    cout << "Starting set commands test..." << endl;
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

        // SADD command
        vector<string> saddArgs = {"SADD", key, member};
        asio::write(*socket, asio::buffer(ConvertToResp(saddArgs)));

        // SMEMBERS/SISMEMBER command - randomly get existing or non-existing key
        uniform_int_distribution<> dist(0, 100);
        if (dist(rng) < 80 && !testKeys.empty()) // 80% chance to get existing key
        {
            auto it = testKeys.begin();
            advance(it, uniform_int_distribution<>(0, testKeys.size() - 1)(rng));
            if (dist(rng) < 50)
            {
                vector<string> smembersArgs = {"SMEMBERS", it->first};
                asio::write(*socket, asio::buffer(ConvertToResp(smembersArgs)));
            }
            else
            {
                vector<string> sismemberArgs = {"SISMEMBER", it->first, it->second[0]};
                asio::write(*socket, asio::buffer(ConvertToResp(sismemberArgs)));
            }
        }
        else // 20% chance to get random key
        {
            string randomKey = GetRandomString(10);
            string randomMember = GetRandomString(20);
            if (dist(rng) < 50)
            {
                vector<string> smembersArgs = {"SMEMBERS", randomKey};
                asio::write(*socket, asio::buffer(ConvertToResp(smembersArgs)));
            }
            else
            {
                vector<string> sismemberArgs = {"SISMEMBER", randomKey, randomMember};
                asio::write(*socket, asio::buffer(ConvertToResp(sismemberArgs)));
            }
        }

        testKeys[key].push_back(member);
    }

    // Cleanup all set keys
    for (const auto& pair : testKeys)
    {
        vector<string> delArgs = {"DEL", pair.first};
        asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
    }

    socket->shutdown(asio::ip::tcp::socket::shutdown_both);
    socket->close();
    cout << "Set commands test completed" << endl;
}