#pragma once
#include <array>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "db.h"
#include "DataStruct/sds.h"
#include "DataType/setobj.hpp"
#include "DataType/hashobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/zsetobj.hpp"
#include "DataType/stringobj.hpp"
#include "Networking/connection.h"

class Server;
extern Server server;

using std::array;
using std::weak_ptr;
using std::shared_ptr;
using std::unordered_set;
using Command = std::vector<Sds*>;
using CommandMap =
    std::unordered_map<std::string, std::function<void(shared_ptr<Connection> conn, Command&)>>;

// string command
bool CmdSet(shared_ptr<Connection> conn, Command& cmd);
bool CmdGet(shared_ptr<Connection> conn, Command& cmd);
bool CmdMset(shared_ptr<Connection> conn, Command& cmd);
bool CmdMget(shared_ptr<Connection> conn, Command& cmd);
bool CmdIncr(shared_ptr<Connection> conn, Command& cmd);
bool CmdDecr(shared_ptr<Connection> conn, Command& cmd);
bool CmdAppend(shared_ptr<Connection> conn, Command& cmd);
bool CmdStrlen(shared_ptr<Connection> conn, Command& cmd);
bool CmdSetnx(shared_ptr<Connection> conn, Command& cmd);
bool CmdSetex(shared_ptr<Connection> conn, Command& cmd);
bool CmdCommand(shared_ptr<Connection> conn, Command& cmd);

// hash command
bool CmdHSet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHDel(shared_ptr<Connection> conn, Command& cmd);
bool CmdHKeys(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGetAll(shared_ptr<Connection> conn, Command& cmd);
bool CmdHLen(shared_ptr<Connection> conn, Command& cmd);
bool CmdHExists(shared_ptr<Connection> conn, Command& cmd);
bool CmdHIncrby(shared_ptr<Connection> conn, Command& cmd);

// list command
bool CmdLPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdLPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdLRange(shared_ptr<Connection> conn, Command& cmd);
bool CmdLLen(shared_ptr<Connection> conn, Command& cmd);
bool CmdLIndex(shared_ptr<Connection> conn, Command& cmd);

// set command
bool CmdSAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdSRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdSPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdSMembers(shared_ptr<Connection> conn, Command& cmd);
bool CmdSisMember(shared_ptr<Connection> conn, Command& cmd);
bool CmdSCard(shared_ptr<Connection> conn, Command& cmd);

// zset command
bool CmdZAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRange(shared_ptr<Connection> conn, Command& cmd);
bool CmdZPopMin(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRevRange(shared_ptr<Connection> conn, Command& cmd);
bool CmdZCard(shared_ptr<Connection> conn, Command& cmd);
bool CmdZScore(shared_ptr<Connection> conn, Command& cmd);

// free command
bool CmdDel(shared_ptr<Connection> conn, Command& cmd);
bool CmdTTL(shared_ptr<Connection> conn, Command& cmd);
bool CmdPing(shared_ptr<Connection> conn, Command& cmd);
bool CmdExpire(shared_ptr<Connection> conn, Command& cmd);
bool CmdKeyNum(shared_ptr<Connection> conn, Command& cmd);
bool CmdFlushDB(shared_ptr<Connection> conn, Command& cmd);
bool CmdFlushAll(shared_ptr<Connection> conn, Command& cmd);

// config command
bool CmdConfigGet(shared_ptr<Connection> conn, Command& cmd);

// Use inline const to avoid multiple copies in each translation unit
inline const std::unordered_map<std::string, std::function<bool(shared_ptr<Connection> conn, Command&)>>
    commands_map = {
        // string command
        {"set", CmdSet},
        {"get", CmdGet},
        {"mset", CmdMset},
        {"mget", CmdMget},
        {"incr", CmdIncr},
        {"decr", CmdDecr},
        {"append", CmdAppend},
        {"strlen", CmdStrlen},
        {"setnx", CmdSetnx},
        {"setex", CmdSetex},
        {"command", CmdCommand},

        // hash command
        {"hset", CmdHSet},
        {"hget", CmdHGet},
        {"hdel", CmdHDel},
        {"hkeys", CmdHKeys},
        {"hgetall", CmdHGetAll},
        {"hlen", CmdHLen},
        {"hexists", CmdHExists},
        {"hincrby", CmdHIncrby},

        // list command
        {"lpop", CmdLPop},
        {"rpop", CmdRPop},
        {"lpush", CmdLPush},
        {"rpush", CmdRPush},
        {"lrange", CmdLRange},
        {"llen", CmdLLen},
        {"lindex", CmdLIndex},

        // set command
        {"sadd", CmdSAdd},
        {"srem", CmdSRem},
        {"spop", CmdSPop},
        {"smembers", CmdSMembers},
        {"sismember", CmdSisMember},
        {"scard", CmdSCard},

        // zset command
        {"zadd", CmdZAdd},
        {"zrem", CmdZRem},
        {"zrange", CmdZRange},
        {"zpopmin", CmdZPopMin},
        {"zrevrange", CmdZRevRange},
        {"zcard", CmdZCard},
        {"zscore", CmdZScore},

        // general command
        {"del", CmdDel},
        {"ttl", CmdTTL},
        {"expire", CmdExpire},
        {"ping", CmdPing},
        {"keynum", CmdKeyNum},
        {"flushall", CmdFlushAll},
        {"config", CmdConfigGet},
    };

inline std::function<bool(shared_ptr<Connection>, Command&)> GetCommandHandler(Sds* cmdtype)
{
    std::string command(cmdtype->buf, cmdtype->length());
    auto it = commands_map.find(command);
    return it != commands_map.end() ? it->second : nullptr;
}

inline unique_ptr<Sds, decltype(&Sds::destroy)> GenerateErrorReply(const char* errmsg)
{
    Sds* reply = Sds::create("-ERR ", 5, 5);
    reply = reply->append(errmsg, strlen(errmsg));
    reply = reply->append("\r\n", 2);

    return {reply, &Sds::destroy};
}

template <typename T>
concept ValidReplyType =
    std::is_same_v<std::remove_cvref_t<T>, std::unique_ptr<ValueRef>> ||
    std::is_same_v<std::remove_cvref_t<T>, std::vector<std::unique_ptr<ValueRef>>>;

// Optimized GenerateReply with pre-allocation
template <typename T>
    requires ValidReplyType<T>
std::unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(T&& result)
{
    std::vector<std::unique_ptr<ValueRef>> vec;
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::unique_ptr<ValueRef>>)
    {
        vec.emplace_back(std::move(result));
    }
    else
    {
        vec = std::move(result);
    }

    // Calculate total size for pre-allocation (only once)
    int size = vec.size();
    std::string size_str = std::to_string(size);
    size_t total_size = 1 + size_str.length() + 2;  // "*" + size + "\r\n"
    for (auto& vr : vec)
    {
        if (vr == nullptr)
        {
            total_size += 6;  // "+nil\r\n"
        }
        else
        {
            std::string len_str = std::to_string(vr->val->length());
            total_size += 1 + len_str.length() + 2 + vr->val->length() + 2;
        }
    }

    // Pre-allocate with enough space
    Sds* reply = Sds::create("", 0, total_size);
    reply = reply->append("*", 1);
    reply = reply->append(size_str.c_str(), size_str.length());
    reply = reply->append("\r\n", 2);

    for (auto& vr : vec)
    {
        if (vr == nullptr)
        {
            reply = reply->append("+nil\r\n", 6);
            continue;
        }

        std::string len_str = std::to_string(vr->val->length());
        reply = reply->append("$", 1);
        reply = reply->append(len_str.c_str(), len_str.length());
        reply = reply->append("\r\n", 2);
        reply = reply->append(vr->val);
        reply = reply->append("\r\n", 2);
    }

    return {reply, &Sds::destroy};
}
