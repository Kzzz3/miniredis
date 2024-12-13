#include "command.h"
#include "server.h"

void CmdLPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (!db->kvstore.contains(cmd[1]))
        db->kvstore[Sds::create(cmd[1])] = ListObjectCreate();

    for (size_t i = 2; i < size; i++)
        ListObjectLPush(db->kvstore[cmd[1]], cmd[i]);
}

void CmdRPush(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (!db->kvstore.contains(cmd[1]))
        db->kvstore[Sds::create(cmd[1])] = ListObjectCreate();
    for (size_t i = 2; i < size; i++)
        ListObjectRPush(db->kvstore[cmd[1]], cmd[i]);
}

void CmdLPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    auto reply = db->kvstore.contains(cmd[1]) ? GenerateReply(ListObjectLPop(db->kvstore[cmd[1]]))
                                              : GenerateErrorReply("nil");
    conn->AsyncSend(std::move(reply));
}

void CmdRPop(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    auto reply = db->kvstore.contains(cmd[1]) ? GenerateReply(ListObjectRPop(db->kvstore[cmd[1]]))
                                              : GenerateErrorReply("nil");
    conn->AsyncSend(std::move(reply));
}

void CmdLRange(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size != 4)
        return;
    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        RedisObj* obj = db->kvstore[cmd[1]];
        LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);

        int start = sds2num<int>(cmd[2]).value();
        int end = sds2num<int>(cmd[3]).value();

        auto reply = GenerateReply(ListObjectRange(obj, start, end));
        conn->AsyncSend(std::move(reply));
    }
    else
    {
        auto reply = GenerateErrorReply("nil");
        conn->AsyncSend(std::move(reply));
    }
}
