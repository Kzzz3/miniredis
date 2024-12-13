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

inline void TestHashCommands()
{
    cout << "Starting hash commands test..." << endl;
    asio::io_context io_context;
    auto socket = make_unique<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));

    unordered_map<string, vector<string>> testKeys;
    static thread_local mt19937 rng{random_device{}()};

    for (int i = 0; i < 10000; i++)
    {
        string key = GetRandomString(10);
        string field = GetRandomString(8);
        string value = GetRandomString(20);

        // HSET command
        vector<string> hsetArgs = {"HSET", key, field, value};
        asio::write(*socket, asio::buffer(ConvertToResp(hsetArgs)));

        // HGET command - randomly get existing or non-existing key/field
        uniform_int_distribution<> dist(0, 100);
        if (dist(rng) < 80 && !testKeys.empty()) // 80% chance to get existing key/field
        {
            auto it = testKeys.begin();
            advance(it, uniform_int_distribution<>(0, testKeys.size() - 1)(rng));
            vector<string> hgetArgs = {"HGET", it->first, it->second[0]};
            asio::write(*socket, asio::buffer(ConvertToResp(hgetArgs)));
        }
        else // 20% chance to get random key/field
        {
            string randomKey = GetRandomString(10);
            string randomField = GetRandomString(8);
            vector<string> hgetArgs = {"HGET", randomKey, randomField};
            asio::write(*socket, asio::buffer(ConvertToResp(hgetArgs)));
        }

        // HGETALL command
        vector<string> hgetallArgs = {"HGETALL", key};
        asio::write(*socket, asio::buffer(ConvertToResp(hgetallArgs)));

        testKeys[key] = {field, value};
    }

    // Cleanup all hash keys
    for (const auto& pair : testKeys)
    {
        vector<string> delArgs = {"DEL", pair.first};
        asio::write(*socket, asio::buffer(ConvertToResp(delArgs)));
    }

    socket->shutdown(asio::ip::tcp::socket::shutdown_both);
    socket->close();
    cout << "Hash commands test completed" << endl;
}