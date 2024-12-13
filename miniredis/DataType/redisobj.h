#pragma once
#include <cstdint>
#include <memory>

constexpr size_t EMBSTR_MAX_LENGTH = 41;

enum class ObjType : uint8_t
{
    REDIS_STRING,
    REDIS_LIST,
    REDIS_HASH,
    REDIS_SET,
    REDIS_ZSET
};

enum class ObjEncoding : uint8_t
{
    REDIS_ENCODING_INT,
    REDIS_ENCODING_EMBSTR,
    REDIS_ENCODING_RAW,
    REDIS_ENCODING_HT,
    REDIS_ENCODING_LINKEDLIST,
    REDIS_ENCODING_ZIPLIST,
    REDIS_ENCODING_INTSET,
    REDIS_ENCODING_RBTREE,
};

#pragma pack(push, 1)
class RedisObj
{
public:
    ObjType type;
    ObjEncoding encoding;

    union
    {
        void* ptr;
        int64_t num;
    } data;
    uint32_t lru;
    uint32_t refcount;
};
#pragma pack(pop)

class ValueRef
{
public:
    Sds* val;
    RedisObj* obj;

    ValueRef(Sds* val, RedisObj* obj) : val(val), obj(obj)
    {
        if (obj != nullptr)
            obj->refcount++;
    }

    ~ValueRef()
    {
        if (obj != nullptr)
            obj->refcount--;
        else
            Sds::destroy(val);
    }
};