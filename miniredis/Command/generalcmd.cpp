#include "command.h"
#include "server.h"

void CmdDel(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 2)
        return;

    RedisDb* db = server.selectDb(cmd[1]);
    if (db->kvstore.contains(cmd[1]))
    {
        auto entry = db->kvstore.find(cmd[1]);

        // recording the key-value pair to be deleted
        Sds* key = entry->first;
        RedisObj* obj = entry->second;

        // delete the key-value pair
        db->kvstore.erase(key);

        // destroy the key and value(object)
        Sds::destroy(key);
        switch (obj->type)
        {
        case ObjType::REDIS_STRING:
            StringObjectDestroy(obj);
            break;
        case ObjType::REDIS_LIST:
            ListObjectDestroy(obj);
            break;
        case ObjType::REDIS_HASH:
            HashObjectDestroy(obj);
            break;
        case ObjType::REDIS_SET:
            SetObjectDestroy(obj);
            break;
        case ObjType::REDIS_ZSET:
            ZsetObjectDestroy(obj);
            break;
        default:
            assert(false);
        }
    }
}

void CmdKeyNum(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return;

    int key_num = 0;
    for (auto& db : server.databases)
    {
        key_num += db.kvstore.size();
    }

    conn->AsyncSend(GenerateReply(make_unique<ValueRef>(num2sds<int>(key_num), nullptr)));
}

void CmdFlushAll(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return;

    for (auto& db : server.databases) // 注意这里加上引用 &
    {
        // 先收集所有需要删除的键
        vector<Sds*> keys_to_delete;
        for (const auto& [key, value] : db.kvstore)
        {
            keys_to_delete.push_back(key);
        }

        // 然后安全地删除每个键值对
        for (auto key : keys_to_delete)
        {
            auto value = db.kvstore[key];
            db.kvstore.erase(key);

            Sds::destroy(key);
            switch (value->type)
            {
            case ObjType::REDIS_STRING:
                StringObjectDestroy(value);
                break;
            case ObjType::REDIS_LIST:
                ListObjectDestroy(value);
                break;
            case ObjType::REDIS_HASH:
                HashObjectDestroy(value);
                break;
            case ObjType::REDIS_SET:
                SetObjectDestroy(value);
                break;
            case ObjType::REDIS_ZSET:
                ZsetObjectDestroy(value);
                break;
            default:
                assert(false);
            }
        }
    }
}