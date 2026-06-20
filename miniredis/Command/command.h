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
bool CmdIncr(shared_ptr<Connection> conn, Command& cmd);
bool CmdDecr(shared_ptr<Connection> conn, Command& cmd);
bool CmdAppend(shared_ptr<Connection> conn, Command& cmd);
bool CmdCommand(shared_ptr<Connection> conn, Command& cmd);

// hash command
bool CmdHSet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHDel(shared_ptr<Connection> conn, Command& cmd);
bool CmdHKeys(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGetAll(shared_ptr<Connection> conn, Command& cmd);

// list command
bool CmdLPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdLPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdLRange(shared_ptr<Connection> conn, Command& cmd);

// set command
bool CmdSAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdSRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdSPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdSMembers(shared_ptr<Connection> conn, Command& cmd);
bool CmdSisMember(shared_ptr<Connection> conn, Command& cmd);

// zset command
bool CmdZAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRange(shared_ptr<Connection> conn, Command& cmd);
bool CmdZPopMin(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRevRange(shared_ptr<Connection> conn, Command& cmd);

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

static std::unordered_map<std::string, std::function<bool(shared_ptr<Connection> conn, Command&)>>
    commands_map = {
        // string command
        {"set", CmdSet},
        {"get", CmdGet},
        {"mset", CmdMset},
        {"incr", CmdIncr},
        {"decr", CmdDecr},
        {"append", CmdAppend},
        {"command", CmdCommand},

        // hash command
        {"hset", CmdHSet},
        {"hget", CmdHGet},
        {"hdel", CmdHDel},
        {"hkeys", CmdHKeys},
        {"hgetall", CmdHGetAll},

        // list command
        {"lpop", CmdLPop},
        {"rpop", CmdRPop},
        {"lpush", CmdLPush},
        {"rpush", CmdRPush},
        {"lrange", CmdLRange},

        // set command
        {"sadd", CmdSAdd},
        {"srem", CmdSRem},
        {"spop", CmdSPop},
        {"smembers", CmdSMembers},
        {"sismember", CmdSisMember},

        // zset command
        {"zadd", CmdZAdd},
        {"zrem", CmdZRem},
        {"zrange", CmdZRange},
        {"zpopmin", CmdZPopMin},
        {"zrevrange", CmdZRevRange},

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
    return commands_map.contains(command) ? commands_map[command] : nullptr;
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

    // Calculate total size for pre-allocation
    size_t total_size = 10;  // "*" + size + "\r\n"
    for (auto& vr : vec)
    {
        if (vr == nullptr)
        {
            total_size += 6;  // "+nil\r\n"
        }
        else
        {
            total_size += 3 + 20 + 2 + vr->val->length() + 2;  // "$" + len + "\r\n" + data + "\r\n"
        }
    }

    // Pre-allocate with enough space
    int size = vec.size();
    std::string size_str = std::to_string(size);
    total_size = 1 + size_str.length() + 2;  // "*" + size + "\r\n"
    for (auto& vr : vec)
    {
        if (vr == nullptr)
        {
            total_size += 6;
        }
        else
        {
            std::string len_str = std::to_string(vr->val->length());
            total_size += 1 + len_str.length() + 2 + vr->val->length() + 2;
        }
    }

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
