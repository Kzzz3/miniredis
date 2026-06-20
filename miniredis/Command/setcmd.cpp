#include "server.h"
#include "command.h"

bool CmdSAdd(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
    if (!kvstore.contains(cmd[1]))
    {
        kvstore[Sds::create(cmd[1])] = SetObjectCreate();
    }

    auto obj = kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    size_t successedNum = 0;
    for (size_t i = 2; i < size; i++)
        successedNum += SetObjectAdd(obj, cmd[i]);
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(successedNum), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSRem(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    size_t successedNum = 0;
    for (size_t i = 2; i < size; i++)
        successedNum += SetObjectRemove(obj, cmd[i]);
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(successedNum), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSPop(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(SetObjectPop(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSMembers(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = GenerateReply(SetObjectMembers(obj));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSisMember(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    auto reply = SetObjectIsMember(obj, cmd[2])
                     ? GenerateReply(make_unique<ValueRef>(Sds::create("true", 4), nullptr))
                     : GenerateReply(make_unique<ValueRef>(Sds::create("false", 5), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdSCard(shared_ptr<Connection> conn, Command& cmd)
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
    if (obj->type != ObjType::REDIS_SET)
    {
        auto reply =
            GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    size_t cardinality = SetObjectCard(obj);
    auto reply = GenerateReply(make_unique<ValueRef>(num2sds(cardinality), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}