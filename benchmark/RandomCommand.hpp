#pragma once
#include <asio.hpp>

#include <vector>
#include <random>
#include <string>
#include <thread>
#include <chrono>

#include "utility.hpp"

using asio::io_context;
using asio::ip::tcp;
using std::atomic;
using std::string;
using std::vector;

using asio::connect;

void RandomCommand(io_context& ctxIo, atomic<bool>& stop)
{
    tcp::socket socket(ctxIo);
    tcp::resolver resolver(ctxIo);
    connect(socket, resolver.resolve("127.0.0.1", "10087"));

    vector<string> vecSetKeys;
    vector<string> vecListKeys;
    vector<string> vecZSetKeys;
    vector<string> vecStringKeys;
    vector<vector<string>> vecHashKeys;

    static thread_local mt19937 rng{random_device{}()};
    uniform_int_distribution<> cmdTypeDist(0, 4);  // 5种数据类型
    uniform_int_distribution<> probDist(0, 99);    // 用于概率判断
    uniform_real_distribution<> keyDist(0.0, 1.0); // 用于决定是否使用已存在的key
    uniform_int_distribution<> scoreDist(0, 1000); // 用于生成zset分数

    auto sendCommand = [&](const vector<string>& args)
    {
        string resp = ConvertToResp(args);
        asio::write(socket, asio::buffer(resp));
        // char reply[1024];
        // size_t len = socket.read_some(asio::buffer(reply, sizeof(reply)));
    };

    while (!stop)
    {
        vector<string> args;
        int cmdType = cmdTypeDist(rng);
        bool useExistingKey = (keyDist(rng) < 0.7);

        switch (cmdType)
        {
        case 0:
        { // String 操作
            int op = probDist(rng) % 5;
            string key;

            if (op == 0 || vecStringKeys.empty() || !useExistingKey)
            {
                // SET 操作或没有已存在的key时
                key = GetRandomString(10);
                args = {"SET", key, GetRandomString(20)};
                vecStringKeys.push_back(key);
            }
            else
            {
                key = vecStringKeys[probDist(rng) % vecStringKeys.size()];
                switch (op)
                {
                case 1:
                    args = {"GET", key};
                    break;
                case 2:
                    args = {"INCR", key};
                    break;
                case 3:
                    args = {"DECR", key};
                    break;
                case 4:
                    args = {"APPEND", key, GetRandomString(5)};
                    break;
                }
            }
            break;
        }

        case 1:
        { // Hash 操作
            int op = probDist(rng) % 5;
            string key, field;

            if (op == 0 || vecHashKeys.empty() || !useExistingKey)
            {
                // HSET 操作或没有已存在的key时
                key = GetRandomString(10);
                field = GetRandomString(8);
                args = {"HSET", key, field, GetRandomString(20)};
                vecHashKeys.push_back({key, field});
            }
            else
            {
                int idx = probDist(rng) % vecHashKeys.size();
                key = vecHashKeys[idx][0];
                field = vecHashKeys[idx][1];

                switch (op)
                {
                case 1:
                    args = {"HGET", key, field};
                    break;
                case 2:
                    args = {"HDEL", key, field};
                    vecHashKeys.erase(vecHashKeys.begin() + idx);
                    break;
                case 3:
                    args = {"HKEYS", key};
                    break;
                case 4:
                    args = {"HGETALL", key};
                    break;
                }
            }
            break;
        }

        case 2:
        { // List 操作
            int op = probDist(rng) % 5;
            string key;

            if ((op <= 1) || vecListKeys.empty() || !useExistingKey)
            {
                key = GetRandomString(10);
                if (op == 0)
                {
                    args = {"LPUSH", key, GetRandomString(20)};
                }
                else
                {
                    args = {"RPUSH", key, GetRandomString(20)};
                }
                if (find(vecListKeys.begin(), vecListKeys.end(), key) == vecListKeys.end())
                {
                    vecListKeys.push_back(key);
                }
            }
            else
            {
                key = vecListKeys[probDist(rng) % vecListKeys.size()];
                switch (op)
                {
                case 2:
                    args = {"LPOP", key};
                    break;
                case 3:
                    args = {"RPOP", key};
                    break;
                case 4:
                {
                    int start = probDist(rng) % 10;
                    int end = start + probDist(rng) % 10;
                    args = {"LRANGE", key, to_string(start), to_string(end)};
                    break;
                }
                }
            }
            break;
        }

        case 3:
        { // Set 操作
            int op = probDist(rng) % 4;
            string key;

            if (op == 0 || vecSetKeys.empty() || !useExistingKey)
            {
                key = GetRandomString(10);
                args = {"SADD", key, GetRandomString(20)};
                if (find(vecSetKeys.begin(), vecSetKeys.end(), key) == vecSetKeys.end())
                {
                    vecSetKeys.push_back(key);
                }
            }
            else
            {
                key = vecSetKeys[probDist(rng) % vecSetKeys.size()];
                switch (op)
                {
                case 1:
                    args = {"SREM", key, GetRandomString(20)};
                    break;
                case 2:
                    args = {"SMEMBERS", key};
                    break;
                case 3:
                    args = {"SISMEMBER", key, GetRandomString(20)};
                    break;
                }
            }
            break;
        }

        case 4:
        { // ZSet 操作
            int op = probDist(rng) % 4;
            string key;

            if (op == 0 || vecZSetKeys.empty() || !useExistingKey)
            {
                key = GetRandomString(10);
                args = {"ZADD", key, to_string(scoreDist(rng)), GetRandomString(20)};
                if (find(vecZSetKeys.begin(), vecZSetKeys.end(), key) == vecZSetKeys.end())
                {
                    vecZSetKeys.push_back(key);
                }
            }
            else
            {
                key = vecZSetKeys[probDist(rng) % vecZSetKeys.size()];
                switch (op)
                {
                case 1:
                    args = {"ZREM", key, GetRandomString(20)};
                    break;
                case 2:
                {
                    int start = probDist(rng) % 10;
                    int end = start + probDist(rng) % 10;
                    args = {"ZRANGE", key, to_string(start), to_string(end)};
                    break;
                }
                case 3:
                {
                    int start = probDist(rng) % 10;
                    int end = start + probDist(rng) % 10;
                    args = {"ZREVRANGE", key, to_string(start), to_string(end)};
                    break;
                }
                }
            }
            break;
        }
        }

        sendCommand(args);
    }

    // clear keys
    for (auto& key : vecStringKeys)
    {
        sendCommand({"DEL", key});
    }
    for (auto& key : vecHashKeys)
    {
        sendCommand({"DEL", key[0]});
    }
    for (auto& key : vecListKeys)
    {
        sendCommand({"DEL", key});
    }
    for (auto& key : vecSetKeys)
    {
        sendCommand({"DEL", key});
    }
    for (auto& key : vecZSetKeys)
    {
        sendCommand({"DEL", key});
    }

    socket.close();
}