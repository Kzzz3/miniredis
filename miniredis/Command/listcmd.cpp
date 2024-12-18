#include "server.h"
#include "command.h"

void CmdLPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (!kvstore.contains(cmd[1]))
        kvstore[Sds::create(cmd[1])] = ListObjectCreate();

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        conn->AsyncSend(GenerateErrorReply(
            "WRONGTYPE Operation against a key holding the wrong kind of value"));
        return;
    }

    for (size_t i = 2; i < size; i++)
        ListObjectLPush(obj, cmd[i]);
}

void CmdRPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (!kvstore.contains(cmd[1]))
        kvstore[Sds::create(cmd[1])] = ListObjectCreate();

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_LIST)
    {
        conn->AsyncSend(GenerateErrorReply(
            "WRONGTYPE Operation against a key holding the wrong kind of value"));
        return;
    }

    for (size_t i = 2; i < size; i++)
        ListObjectRPush(obj, cmd[i]);
}

void CmdLPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_LIST)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = GenerateReply(ListObjectLPop(obj));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdRPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_LIST)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = GenerateReply(ListObjectRPop(obj));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdLRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return;
    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_LIST)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        int start = sds2num<int>(cmd[2]).value();
        int end = sds2num<int>(cmd[3]).value();

        auto reply = GenerateReply(ListObjectRange(obj, start, end));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}
