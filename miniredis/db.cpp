#include "db.h"

RedisDb::RedisDb()
{
    kvstore = Allocator::create<HashTable<RedisObj*>>();
    deadobj = Allocator::create<std::list<RedisObj*>>();
}

RedisDb::~RedisDb()
{
    Allocator::destroy(kvstore);
    Allocator::destroy(deadobj);
}

void RedisDb::serialize_to(ofstream& ofs)
{

    constexpr auto seri_func = [](ofstream& ofs, RedisObj* obj)
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
    };
    HashTable<RedisObj*>::serialize_to(ofs, kvstore, seri_func);
}

void RedisDb::deserialize_from(ifstream& ifs)
{
    constexpr auto deser_func = [](ifstream& ifs) -> RedisObj*
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
    };
    kvstore = HashTable<RedisObj*>::deserialize_from(ifs, deser_func);
}
