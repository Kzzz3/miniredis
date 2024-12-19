#include "db.h"

uint32_t DATABASE_NUM = 16;

RedisDb::RedisDb(thread_pool& work_executor, io_context& io_context)
    : rdb_timer(io_context), delobj_timer(io_context)
{
    for (int i = 0; i < DATABASE_NUM; ++i)
    {
        kvstores.emplace_back();
        expired_kvstores.emplace_back();
    }
    co_spawn(work_executor, rdbTimerHandler(), detached);
    co_spawn(work_executor, delObjectHandler(), detached);
}

RedisDb::~RedisDb()
{
    // destroy kvstore
    for (auto& kvstore : kvstores)
    {
        for (auto& [key, value] : kvstore)
        {
            // destroy key
            Sds::destroy(key);

            value->refcount--;
            if (value->refcount == 0)
                destroyRedisObj(value);
            else
                deadobj.push_back(value);
        }
        kvstore.clear();
    }
}

awaitable<void> RedisDb::rdbTimerHandler()
{
    while (true)
    {
        rdb_timer.expires_after(std::chrono::seconds(RDB_TIMER_INTERVAL));
        co_await rdb_timer.async_wait(use_awaitable);

        storeRDB("rdb.dat");
        std::cout << "storing rdb..." << std::endl;
        CompressFileStream("rdb.dat", "rdb.dat.gz");
        std::cout << "storing rdb done" << std::endl;
    }
}

awaitable<void> RedisDb::delObjectHandler()
{
    while (true)
    {
        delobj_timer.expires_after(std::chrono::seconds(DEL_TIMER_INTERVAL));
        co_await delobj_timer.async_wait(use_awaitable);

        for (auto& obj : deadobj)
        {
            if (obj->refcount == 0)
            {
                destroyRedisObj(obj);
                deadobj.remove(obj);
            }
        }
    }
}

HashTable<RedisObj*>& RedisDb::getKVStore(Sds* key)
{
    return kvstores[std::hash<Sds*>{}(key) % DATABASE_NUM];
}

HashTable<RedisObj*>& RedisDb::getExpiredKVStore(Sds* key)
{
    return expired_kvstores[std::hash<Sds*>{}(key) % DATABASE_NUM];
}

void RedisDb::loadRDB(const string& path)
{
    ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs)
        throw std::runtime_error("open rdb file failed");

    for (auto& kvstore : kvstores)
    {
        kvstore = std::move(*HashTable<RedisObj*>::deserialize_from(ifs, &valueDeserializeFunc));
    }
}

void RedisDb::storeRDB(const string& path)
{
    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    ofstream ofs(path, std::ios::out | std::ios::binary);
    if (!ofs)
        throw std::runtime_error("open rdb file failed");

    for (auto& kvstore : kvstores)
    {
        HashTable<RedisObj*>::serialize_to(ofs, &kvstore, &valueSerializeFunc);
    }
}

void RedisDb::destroyRedisObj(RedisObj* obj)
{
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

RedisObj* RedisDb::valueDeserializeFunc(ifstream& ifs)
{
    RedisObj* obj = Allocator::create<RedisObj>();

    auto expect_type = struct_pack::deserialize<ObjType>(ifs);
    if (!expect_type.has_value())
    {
        std::cout << expect_type.error().message() << std::endl;
        throw std::runtime_error("deserialize failed");
    }
    obj->type = expect_type.value();

    auto expect_encoding = struct_pack::deserialize<ObjEncoding>(ifs);
    if (!expect_encoding.has_value())
        throw std::runtime_error("deserialize failed");
    obj->encoding = expect_encoding.value();

    switch (obj->type)
    {
    case ObjType::REDIS_STRING:
        StringObjectDataDeserialize(ifs, obj);
        break;
    case ObjType::REDIS_LIST:
        ListObjectDataDeserialize(ifs, obj);
        break;
    case ObjType::REDIS_SET:
        SetObjectDataDeserialize(ifs, obj);
        break;
    case ObjType::REDIS_ZSET:
        ZsetObjectDataDeserialize(ifs, obj);
        break;
    case ObjType::REDIS_HASH:
        HashObjectDataDeserialize(ifs, obj);
        break;
    default:
        throw std::runtime_error("invalid type");
    }

    auto expect_lru = struct_pack::deserialize<uint32_t>(ifs);
    if (!expect_lru.has_value())
        throw std::runtime_error("deserialize failed");
    obj->lru = expect_lru.value();

    auto expect_refcount = struct_pack::deserialize<uint32_t>(ifs);
    if (!expect_refcount.has_value())
        throw std::runtime_error("deserialize failed");
    obj->refcount = expect_refcount.value();

    return obj;
}

void RedisDb::valueSerializeFunc(ofstream& ofs, RedisObj* obj)
{
    struct_pack::serialize_to(ofs, obj->type);
    struct_pack::serialize_to(ofs, obj->encoding);

    switch (obj->type)
    {
    case ObjType::REDIS_STRING:
        StringObjectDataSerialize(ofs, obj);
        break;
    case ObjType::REDIS_LIST:
        ListObjectDataSerialize(ofs, obj);
        break;
    case ObjType::REDIS_SET:
        SetObjectDataSerialize(ofs, obj);
        break;
    case ObjType::REDIS_ZSET:
        ZsetObjectDataSerialize(ofs, obj);
        break;
    case ObjType::REDIS_HASH:
        HashObjectDataSerialize(ofs, obj);
        break;
    default:
        throw std::runtime_error("invalid type");
    }

    struct_pack::serialize_to(ofs, obj->lru);
    struct_pack::serialize_to(ofs, obj->refcount);
}