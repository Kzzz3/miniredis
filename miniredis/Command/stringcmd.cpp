#include "server.h"
#include "command.h"

bool CmdSet(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    // Check memory limit before inserting
    if (MAXMEMORY > 0 && server.database.getMemoryUsage() > MAXMEMORY)
    {
        server.database.evictLRU();
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[cmd[1]] = StringObjectCreate(cmd[2]);
        cmd[1] = nullptr;

        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_STRING)
    {
        RedisObjDestroy(obj);
        kvstore[cmd[1]] = StringObjectCreate(cmd[2]);
    }
    else
    {
        kvstore[cmd[1]] = StringObjectUpdate(obj, cmd[2]);
    }

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdGet(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    // Check if key is expired (lazy deletion)
    if (server.database.isKeyExpired(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("nil"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("nil"), nullptr));
        conn->AsyncSend(std::move(reply));
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

    // Update LRU timestamp
    obj->lru = GetSecTimestamp();

    auto reply = GenerateReply(StringObjectGet(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdMset(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() < 3 || cmd.size() % 2 == 0)
        return false;

    for (size_t i = 1; i < cmd.size(); i += 2)
    {
        HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[i]);
        if (!kvstore.contains(cmd[i]))
        {
            kvstore[cmd[i]] = StringObjectCreate(cmd[i + 1]);
            cmd[i] = nullptr;
            continue;
        }

        auto obj = kvstore[cmd[i]];
        if (obj->type != ObjType::REDIS_STRING)
        {
            RedisObjDestroy(obj);
            kvstore[cmd[i]] = StringObjectCreate(cmd[i + 1]);
        }
        else
        {
            kvstore[cmd[i]] = StringObjectUpdate(obj, cmd[i + 1]);
        }
    }

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
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
        Sds* value = Sds::create("1");
        kvstore[cmd[1]] = StringObjectCreate(value);
        cmd[1] = nullptr;

        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("1"), nullptr));
        conn->AsyncSend(std::move(reply));
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

    if (obj->encoding != ObjEncoding::REDIS_ENCODING_INT)
    {
        auto reply = GenerateErrorReply("value is not an integer");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    obj->data.num++;
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(obj->data.num), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdDecr(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        Sds* value = Sds::create("-1");
        kvstore[cmd[1]] = StringObjectCreate(value);
        cmd[1] = nullptr;

        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("-1"), nullptr));
        conn->AsyncSend(std::move(reply));
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

    if (obj->encoding != ObjEncoding::REDIS_ENCODING_INT)
    {
        auto reply = GenerateErrorReply("value is not an integer");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    obj->data.num--;
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(obj->data.num), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdAppend(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("key doesn't exist");
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

bool CmdCommand(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}