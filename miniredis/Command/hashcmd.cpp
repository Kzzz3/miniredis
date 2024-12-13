#include "command.h"
#include "server.h"

void CmdHSet(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 4 || size % 2 != 0)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (!db->kvstore.contains(cmd[1]))
    {
        db->kvstore[Sds::create(cmd[1])] = HashObjectCreate();
    }

    RedisObj* hashobj = db->kvstore[cmd[1]];
    for (size_t i = 2; i < size; i += 2)
    {
        HashObjectInsert(hashobj, cmd[i], cmd[i + 1]);
    }
}

void CmdHGet(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        vector<unique_ptr<ValueRef>> result;
        RedisObj* hashobj = db->kvstore[cmd[1]];

        for (size_t i = 2; i < size; i++)
            result.push_back(HashObjectGet(hashobj, cmd[i]));

        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}

void CmdHDel(shared_ptr<Connection> conn, Command& cmd)
{
    size_t size = cmd.size();
    if (size < 3)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        RedisObj* hashobj = db->kvstore[cmd[1]];
        for (size_t i = 2; i < size; i++)
            HashObjectRemove(hashobj, cmd[i]);
    }
}

void CmdHKeys(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        RedisObj* hashobj = db->kvstore[cmd[1]];
        vector<unique_ptr<ValueRef>> result = HashObjectKeys(hashobj);

        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}

void CmdHGetAll(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        RedisObj* hashobj = db->kvstore[cmd[1]];
        vector<unique_ptr<ValueRef>> result = HashObjectKVs(hashobj);
        conn->AsyncSend(GenerateReply(result));
    }
    else
    {
        conn->AsyncSend(GenerateErrorReply("nil"));
    }
}