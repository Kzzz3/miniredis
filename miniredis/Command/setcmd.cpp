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

    for (size_t i = 2; i < size; i++)
        SetObjectAdd(db->kvstore[cmd[1]], cmd[i]);
}

void CmdSRem(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        for (size_t i = 2; i < size; i++)
            SetObjectRemove(db->kvstore[cmd[1]], cmd[i]);
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
        auto reply = GenerateReply(SetObjectMembers(db->kvstore[cmd[1]]));
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
        auto reply = SetObjectIsMember(db->kvstore[cmd[1]], cmd[2])
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