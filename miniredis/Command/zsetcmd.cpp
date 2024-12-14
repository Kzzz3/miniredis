#include "command.h"
#include "server.h"

void CmdZAdd(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 4 || size % 2 != 0)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (!db->kvstore.contains(cmd[1]))
    {
        db->kvstore[Sds::create(cmd[1])] = ZsetObjectCreate();
    }

    auto obj = db->kvstore[cmd[1]];
    if (obj->type != ObjType::REDIS_ZSET)
    {
        conn->AsyncSend(GenerateErrorReply(
            "WRONGTYPE Operation against a key holding the wrong kind of value"));
        return;
    }

    for (size_t i = 2; i < size; i += 2)
    {
        ZsetObjectAdd(obj, sds2num<double>(cmd[i]).value(), cmd[i + 1]);
    }
}

void CmdZRem(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_ZSET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        for (size_t i = 2; i < size; i++)
            ZsetObjectRemove(obj, cmd[i]);
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdZRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_ZSET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = GenerateReply(
            ZsetObjectRange(obj, sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}

void CmdZRevRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto obj = db->kvstore[cmd[1]];
        if (obj->type != ObjType::REDIS_ZSET)
        {
            conn->AsyncSend(GenerateErrorReply(
                "WRONGTYPE Operation against a key holding the wrong kind of value"));
            return;
        }
        auto reply = GenerateReply(ZsetObjectRevRange(obj, sds2num<double>(cmd[2]).value(),
                                                      sds2num<double>(cmd[3]).value()));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}
