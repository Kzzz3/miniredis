#pragma once

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/zset.h"

inline RedisObj* CreateZsetObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_ZSET;
	obj->encoding = ObjEncoding::REDIS_ENCODING_SKIPLIST;
	obj->data.ptr = std::malloc(sizeof(ZSet));
	new (obj->data.ptr) ZSet();
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}