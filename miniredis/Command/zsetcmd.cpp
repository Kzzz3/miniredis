#include "server.h"
#include "command.h"

bool CmdZAdd(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 4 || size % 2 != 0)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[Sds::create(cmd[1])] = ZsetObjectCreate();
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_ZSET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 2; i < size; i += 2)
    {
        ZsetObjectAdd(obj, sds2num<double>(cmd[i]).value(), cmd[i + 1]);
    }
    return true;
}

bool CmdZRem(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_ZSET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 2; i < size; i++)
        ZsetObjectRemove(obj, cmd[i]);
    return true;
}

bool CmdZRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_ZSET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(
        ZsetObjectRange(obj, sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdZRevRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_ZSET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(
        ZsetObjectRevRange(obj, sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
    conn->AsyncSend(std::move(reply));
    return true;
}