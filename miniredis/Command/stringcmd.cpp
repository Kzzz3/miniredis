#include "server.h"
#include "command.h"

bool CmdSet(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[Sds::create(cmd[1])] = StringObjectCreate(cmd[2]);
        return true;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    obj->data.ptr = StringObjectUpdate(obj, cmd[2]);
    return true;
}

bool CmdGet(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(StringObjectGet(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdIncr(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    if (obj->encoding != ObjEncoding::REDIS_ENCODING_INT)
    {
        auto reply = GenerateErrorReply("value is not an integer");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    obj->data.num++;
    return true;
}

bool CmdDecr(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    if (obj->encoding != ObjEncoding::REDIS_ENCODING_INT)
    {
        auto reply = GenerateErrorReply("value is not an integer");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    obj->data.num--;
    return true;
}

bool CmdAppend(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    if (obj->encoding != ObjEncoding::REDIS_ENCODING_RAW)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto value = reinterpret_cast<Sds*>(obj->data.ptr);
    obj->data.ptr = value->append(cmd[2]);
    return true;
}