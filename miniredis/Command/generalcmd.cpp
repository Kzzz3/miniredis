#include "command.h"
#include "server.h"

bool CmdDel(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    HashTable<RedisObj*>& expired_kvstore = server.database.getExpiredKVStore(cmd[1]);
    std::list<RedisObj*>& deadobj = server.database.deadobj;

    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto entry = kvstore.find(cmd[1]);

    // recording the key-value pair to be deleted
    Sds* key = entry->first;
    RedisObj* obj = entry->second;

    // delete the key-value pair
    kvstore.erase(key);
    Sds::destroy(key);

    // delete expiration if exists
    if (expired_kvstore.contains(cmd[1]))
    {
        RedisObj* expire_obj = expired_kvstore[cmd[1]];
        expired_kvstore.erase(cmd[1]);
        Allocator::destroy(expire_obj);
    }

    obj->refcount--;
    deadobj.push_back(obj);
    return true;
}

bool CmdTTL(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    HashTable<RedisObj*>& expired_kvstore = server.database.getExpiredKVStore(cmd[1]);

    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("-2"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    if (!expired_kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("-1"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    // Get expiration time from expired_kvstore
    RedisObj* expire_obj = expired_kvstore[cmd[1]];
    uint64_t expire_time = expire_obj->data.num;
    uint64_t now = GetSecTimestamp();

    if (now >= expire_time)
    {
        // Key has expired, delete it
        auto entry = kvstore.find(cmd[1]);
        Sds* key = entry->first;
        RedisObj* obj = entry->second;

        kvstore.erase(key);
        expired_kvstore.erase(cmd[1]);
        Sds::destroy(key);

        obj->refcount--;
        if (obj->refcount == 0)
            RedisObjDestroy(obj);
        else
            server.database.deadobj.push_back(obj);

        // Also destroy expire object
        Allocator::destroy(expire_obj);

        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("-2"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    int64_t ttl = expire_time - now;
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(ttl), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdExpire(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    HashTable<RedisObj*>& expired_kvstore = server.database.getExpiredKVStore(cmd[1]);

    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    auto seconds = str2num<int64_t>(cmd[2]->buf, cmd[2]->length());
    if (!seconds.has_value() || seconds.value() <= 0)
    {
        auto reply = GenerateErrorReply("ERR invalid expire time");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    uint64_t expire_time = GetSecTimestamp() + seconds.value();

    // Store expiration time
    if (expired_kvstore.contains(cmd[1]))
    {
        // Update existing expiration
        RedisObj* expire_obj = expired_kvstore[cmd[1]];
        expire_obj->data.num = expire_time;
    }
    else
    {
        // Create new expiration entry
        RedisObj* expire_obj = Allocator::create<RedisObj>();
        expire_obj->type = ObjType::REDIS_STRING;
        expire_obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
        expire_obj->data.num = expire_time;
        expire_obj->lru = GetSecTimestamp();
        expire_obj->refcount = 1;
        expired_kvstore[cmd[1]] = expire_obj;
    }

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("1"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdPing(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("PONG"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdKeyNum(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    int key_num = 0;
    for (auto& kvstore : server.database.kvstores)
    {
        key_num += kvstore.size();
    }

    auto reply = GenerateReply(make_unique<ValueRef>(num2sds<int>(key_num), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdFlushAll(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    for (auto& kvstore : server.database.kvstores)
    {
        for (const auto& [key, value] : kvstore)
        {
            Sds::destroy(key);

            value->refcount--;
            if (value->refcount == 0)
            {
                RedisObjDestroy(value);
            }
            else
            {
                server.database.deadobj.push_back(value);
            }
        }
        kvstore.clear();
    }

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdConfigGet(shared_ptr<Connection> conn, Command& cmd)
{
    std::vector<std::unique_ptr<ValueRef>> vec;
    if (cmd[2]->strcmp("save") == 0)
    {
        vec.emplace_back(make_unique<ValueRef>(Sds::create("save"), nullptr));
        vec.emplace_back(make_unique<ValueRef>(Sds::create("3600 1 300 100 60 10000"), nullptr));
    }
    else if (cmd[2]->strcmp("appendonly") == 0)
    {
        vec.emplace_back(make_unique<ValueRef>(Sds::create("appendonly"), nullptr));
        vec.emplace_back(make_unique<ValueRef>(Sds::create("no"), nullptr));
    }
    else
    {
        auto reply = GenerateErrorReply("ERR unknown config parameter");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(std::move(vec));
    conn->AsyncSend(std::move(reply));
    return true;
}