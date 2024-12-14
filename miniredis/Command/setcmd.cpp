#include "command.h"
#include "server.h"

void CmdSAdd(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (!db->kvstore.contains(cmd[1]))
    {
        db->kvstore[Sds::create(cmd[1])] = SetObjectCreate();
    }

    auto obj = db->kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_SET)
    {
        conn->AsyncSend(GenerateErrorReply(
            "WRONGTYPE Operation against a key holding the wrong kind of value"));
        return;
    }

    for (size_t i = 2; i < size; i++)
        SetObjectAdd(obj, cmd[i]);
}

void CmdSRem(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_SET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        for (size_t i = 2; i < size; i++)
            SetObjectRemove(obj, cmd[i]);
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdSMembers(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return;
    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_SET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = GenerateReply(SetObjectMembers(obj));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdSisMember(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_SET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = SetObjectIsMember(obj, cmd[2])
                         ? GenerateReply(make_unique<ValueRef>(Sds::create("true", 4), nullptr))
                         : GenerateReply(make_unique<ValueRef>(Sds::create("false", 5), nullptr));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}