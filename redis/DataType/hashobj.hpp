#pragma once

#include <memory>

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/dict.h"

using std::malloc;

inline RedisObj* CreateHashObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_HASH;
	obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
	obj->data.ptr = std::malloc(sizeof(Dict));
	new (obj->data.ptr) Dict();
	return obj;
}

constexpr void HashObjInsert(RedisObj* obj, Sds* key)
{

}