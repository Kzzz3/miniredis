#pragma once 
#include <memory>
#include <cstdint>

#include "../utility.h"
#include "../DataStruct/sds.h"

constexpr size_t EMBSTR_MAX_LENGTH = 41;

enum class ObjType 
	:uint8_t
{
	REDIS_STRING,
	REDIS_LIST,
	REDIS_HASH,
	REDIS_SET,
	REDIS_ZSET
};

enum class ObjEncoding 
	:uint8_t
{
	REDIS_ENCODING_INT,
	REDIS_ENCODING_EMBSTR,
	REDIS_ENCODING_RAW,
	REDIS_ENCODING_HT,
	REDIS_ENCODING_LINKEDLIST,
	REDIS_ENCODING_ZIPLIST,
	REDIS_ENCODING_INTSET,
	REDIS_ENCODING_SKIPLIST,
};

#pragma pack(push, 1)
class RedisObj
{
public:
	ObjType type;
	ObjEncoding encoding;

	void* ptr;
	uint32_t lru;
	uint32_t refcount;
};
#pragma pack(pop)

RedisObj* CreateStringObject(const char* str, size_t len);

RedisObj* CreateHashObject(const char* field, const char* value, size_t field_len, size_t value_len);

RedisObj* CreateListObject(const char* value, size_t len);

RedisObj* CreateSetObject(const char* member, size_t len);

RedisObj* CreateZsetObject(const char* member, double score);