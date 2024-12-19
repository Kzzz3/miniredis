#include "server.h"
#include "command.h"

bool CmdLPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
        kvstore[Sds::create(cmd[1])] = ListObjectCreate();

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 2; i < size; i++)
        ListObjectLPush(obj, cmd[i]);
    return true;
}

bool CmdRPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
        kvstore[Sds::create(cmd[1])] = ListObjectCreate();

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 2; i < size; i++)
        ListObjectRPush(obj, cmd[i]);
    return true;
}

bool CmdLPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(ListObjectLPop(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdRPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(ListObjectRPop(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdLRange(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_LIST)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    int start = sds2num<int>(cmd[2]).value();
    int end = sds2num<int>(cmd[3]).value();

    auto reply = GenerateReply(ListObjectRange(obj, start, end));
    conn->AsyncSend(std::move(reply));
    return true;
}
