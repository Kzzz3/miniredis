export module redisobj;

import std;
import sds;
import utility;

using std::byte;
using std::uint8_t;
using std::uint32_t;
using std::uint64_t;

export constexpr size_t EMBSTR_MAX_LENGTH = 41;

export enum class ObjType :uint8_t
{
	REDIS_STRING,
	REDIS_LIST,
	REDIS_HASH,
	REDIS_SET,
	REDIS_ZSET
};

export enum class ObjEncoding :uint8_t
{
	REDIS_ENCODING_INT,
	REDIS_ENCODING_EMBSTR,
	REDIS_ENCODING_RAW,
	REDIS_ENCODING_HT,
	REDIS_ENCODING_LINKEDLIST,
	REDIS_ENCODING_ZIPLIST,
	REDIS_ENCODING_INTSET,
	REDIS_ENCODING_SKIPLIST
};

#pragma pack(push, 1)
export class RedisObj
{
public:
	ObjType type;
	ObjEncoding encoding;

	void* ptr;
	uint64_t lru;
	uint32_t refcount;
};
#pragma pack(pop)

export RedisObj* CreateStringObject(const char* str, size_t len);

export RedisObj* CreateHashObject(const char* field, const char* value, size_t field_len, size_t value_len);

export RedisObj* CreateListObject(const char* value, size_t len);

export RedisObj* CreateSetObject(const char* member, size_t len);

export RedisObj* CreateZsetObject(const char* member, double score);