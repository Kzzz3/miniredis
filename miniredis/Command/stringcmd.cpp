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

bool CmdStrlen(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    // Check if key is expired
    if (server.database.isKeyExpired(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
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

    // Update LRU
    obj->lru = GetSecTimestamp();

    size_t len = 0;
    if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
    {
        // Convert number to string and get length
        Sds* sds = num2sds(obj->data.num);
        len = sds->length();
        Sds::destroy(sds);
    }
    else
    {
        Sds* sds = reinterpret_cast<Sds*>(obj->data.ptr);
        len = sds->length();
    }

    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(len), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdMget(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() < 2)
        return false;

    std::vector<std::unique_ptr<ValueRef>> vec;
    for (size_t i = 1; i < cmd.size(); ++i)
    {
        // Check if key is expired
        if (server.database.isKeyExpired(cmd[i]))
        {
            vec.emplace_back(nullptr);
            continue;
        }

        HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[i]);
        if (!kvstore.contains(cmd[i]))
        {
            vec.emplace_back(nullptr);
            continue;
        }

        auto obj = kvstore[cmd[i]];
        if (obj->type != ObjType::REDIS_STRING)
        {
            vec.emplace_back(nullptr);
            continue;
        }

        // Update LRU
        obj->lru = GetSecTimestamp();
        vec.emplace_back(StringObjectGet(obj));
    }

    auto reply = GenerateReply(std::move(vec));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSetnx(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    // Check memory limit
    if (MAXMEMORY > 0 && server.database.getMemoryUsage() > MAXMEMORY)
    {
        server.database.evictLRU();
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (kvstore.contains(cmd[1]))
    {
        // Key exists, return 0
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    // Key doesn't exist, set it
    kvstore[cmd[1]] = StringObjectCreate(cmd[2]);
    cmd[1] = nullptr;

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("1"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSetex(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 4)
        return false;

    // Check memory limit
    if (MAXMEMORY > 0 && server.database.getMemoryUsage() > MAXMEMORY)
    {
        server.database.evictLRU();
    }

    // Parse seconds
    auto seconds = str2num<int64_t>(cmd[2]->buf, cmd[2]->length());
    if (!seconds.has_value() || seconds.value() <= 0)
    {
        auto reply = GenerateErrorReply("ERR invalid expire time");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    HashTable<RedisObj*>& expired_kvstore = server.database.getExpiredKVStore(cmd[1]);

    // Delete old key if exists
    if (kvstore.contains(cmd[1]))
    {
        auto obj = kvstore[cmd[1]];
        kvstore.erase(cmd[1]);
        obj->refcount--;
        if (obj->refcount == 0)
            RedisObjDestroy(obj);
        else
            server.database.deadobj.push_back(obj);
    }

    // Set new value
    kvstore[cmd[1]] = StringObjectCreate(cmd[3]);
    cmd[1] = nullptr;

    // Set expiration
    uint64_t expire_time = GetSecTimestamp() + seconds.value();
    RedisObj* expire_obj = Allocator::create<RedisObj>();
    expire_obj->type = ObjType::REDIS_STRING;
    expire_obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
    expire_obj->data.num = expire_time;
    expire_obj->lru = GetSecTimestamp();
    expire_obj->refcount = 1;
    expired_kvstore[cmd[1]] = expire_obj;

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}