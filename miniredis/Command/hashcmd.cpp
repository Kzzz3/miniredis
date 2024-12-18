#include "server.h"
#include "command.h"

void CmdHSet(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 4 || size % 2 != 0)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[Sds::create(cmd[1])] = HashObjectCreate();
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_HASH)
    {
        conn->AsyncSend(GenerateErrorReply(
            "WRONGTYPE Operation against a key holding the wrong kind of value"));
        return;
    }

    for (size_t i = 2; i < size; i += 2)
    {
        HashObjectInsert(obj, cmd[i], cmd[i + 1]);
    }
}

void CmdHGet(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_HASH)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }

        vector<unique_ptr<ValueRef>> result;
        for (size_t i = 2; i < size; i++)
            result.push_back(HashObjectGet(obj, cmd[i]));
        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}

void CmdHDel(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_HASH)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        for (size_t i = 2; i < size; i++)
            HashObjectRemove(obj, cmd[i]);
    }
}

void CmdHKeys(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_HASH)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }

        vector<unique_ptr<ValueRef>> result = HashObjectKeys(obj);
        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}

void CmdHGetAll(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    HashTable<RedisObj*>& kvstore = *db->kvstore;
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_HASH)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }

        vector<unique_ptr<ValueRef>> result = HashObjectKVs(obj);
        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}