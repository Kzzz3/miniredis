#include "server.h"
#include "command.h"

bool CmdHSet(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 4 || size % 2 != 0)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[Sds::create(cmd[1])] = HashObjectCreate();
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    size_t successedNum = 0;
    for (size_t i = 2; i < size; i += 2)
        successedNum += HashObjectInsert(obj, cmd[i], cmd[i + 1]);
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(successedNum), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdHGet(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    vector<unique_ptr<ValueRef>> result;
    for (size_t i = 2; i < size; i++)
        result.push_back(HashObjectGet(obj, cmd[i]));
    conn->AsyncSend(GenerateReply(result));
    return true;
}

bool CmdHDel(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 2; i < size; i++)
        HashObjectRemove(obj, cmd[i]);
    return true;
}

bool CmdHKeys(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    vector<unique_ptr<ValueRef>> result = HashObjectKeys(obj);
    conn->AsyncSend(GenerateReply(result));
    return true;
}

bool CmdHGetAll(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    vector<unique_ptr<ValueRef>> result = HashObjectKVs(obj);
    conn->AsyncSend(GenerateReply(result));
    return true;
}

bool CmdHLen(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    size_t len = HashObjectLen(obj);
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(len), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdHExists(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("0"), nullptr));
        conn->AsyncSend(std::move(reply));
        return true;
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    bool exists = HashObjectExists(obj, cmd[2]);
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create(exists ? "1" : "0"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdHIncrby(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 4)
        return false;

    // Parse increment value
    auto increment = str2num<int64_t>(cmd[3]->buf, cmd[3]->length());
    if (!increment.has_value())
    {
        auto reply = GenerateErrorReply("ERR value is not an integer or out of range");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        // Create new hash
        kvstore[Sds::create(cmd[1])] = HashObjectCreate();
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_HASH)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    // Get current value
    auto current = HashObjectGet(obj, cmd[2]);
    int64_t current_value = 0;
    if (current && current->val)
    {
        auto parsed = str2num<int64_t>(current->val->buf, current->val->length());
        if (parsed.has_value())
            current_value = parsed.value();
    }

    // Update value
    int64_t new_value = current_value + increment.value();
    Sds* new_value_str = num2sds(new_value);
    HashObjectInsert(obj, cmd[2], new_value_str);
    Sds::destroy(new_value_str);

    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(new_value), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}
