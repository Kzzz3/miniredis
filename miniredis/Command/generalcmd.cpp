#include "command.h"
#include "server.h"

bool CmdDel(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return false;

    HashTable<RedisObj*>& kvstore = server.database.getKVStore(cmd[1]);
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

    obj->refcount--;
    deadobj.push_back(obj);
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