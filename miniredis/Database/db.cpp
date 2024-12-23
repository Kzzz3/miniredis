#include "db.h"
#include "server.h"

RedisDb::RedisDb(asio::thread_pool& work_executor, asio::io_context& io_context)
    : work_executor(work_executor), io_context(io_context), rdb_timer(io_context),
      delobj_timer(io_context), aof(work_executor, io_context)
{
    for (int i = 0; i < DATABASE_NUM; ++i)
    {
        kvstores.emplace_back();
        expired_kvstores.emplace_back();
    }

    // RDB and AOF cannot be enabled at the same time in this version
    assert(!RDB_ENABLED || !AOF_ENABLED);
    loadPersistedData();

    co_spawn(work_executor, delObjectHandler(), detached);
    startDataPersistence();
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
                RedisObjDestroy(value);
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
                RedisObjDestroy(obj);
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

void RedisDb::loadPersistedData()
{
    if (RDB_ENABLED)
    {
        if (std::filesystem::exists("rdb.dat.gz"))
        {
            std::cout << "decompress rdb.dat.gz..." << std::endl;
            DecompressFileStream("rdb.dat.gz", "rdb.dat");
            std::cout << "decompress rdb.dat.gz done" << std::endl;
        }

        if (std::filesystem::exists("rdb.dat"))
        {
            std::cout << "loading rdb..." << std::endl;
            loadRDB("rdb.dat");
            std::cout << "loading rdb done" << std::endl;
        }
    }

    if (AOF_ENABLED)
    {
        if (std::filesystem::exists("aof.dat.gz"))
        {
            std::cout << "decompress aof.dat.gz..." << std::endl;
            DecompressFileStream("aof.dat.gz", "aof.dat");
            std::cout << "decompress aof.dat.gz done" << std::endl;
        }

        if (std::filesystem::exists("aof.dat"))
        {
            std::cout << "loading aof..." << std::endl;
            aof.loadAOF("aof.dat");
            std::cout << "loading aof done" << std::endl;
        }

        asio::post(work_executor,
                   [this]()
                   {
                       shared_ptr<Connection> temp_conn =
                           make_shared<Connection>(-1, tcp::socket(io_context));
                       temp_conn->Close();

                       for (auto& cmd : aof.aof_cmds)
                       {
                           GetCommandHandler(cmd[0])(temp_conn, cmd);
                       }

                       for (auto& cmd : aof.aof_cmds)
                       {
                           for (auto& sds : cmd)
                           {
                               Sds::destroy(sds);
                           }
                       }
                       aof.aof_cmds.clear();
                   });
    }
}

void RedisDb::startDataPersistence()
{
    if (RDB_ENABLED)
        startRdb();
    if (AOF_ENABLED)
        aof.startAof();
}

void RedisDb::startRdb()
{
    co_spawn(work_executor, rdbTimerHandler(), detached);
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