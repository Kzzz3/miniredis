#pragma once

#include "redisobj.h"
#include "../DataStruct/dict.h"

inline RedisObj* CreateSetObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_SET;
	obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
	obj->data.ptr = std::malloc(sizeof(Dict));
	new (obj->data.ptr) Dict();
	return obj;
}