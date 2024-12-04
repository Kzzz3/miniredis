#pragma once

#include "redisobj.h"
#include "../DataStruct/linkedlist.h"

inline RedisObj* CreateListObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_LIST;
	obj->encoding = ObjEncoding::REDIS_ENCODING_LINKEDLIST;
	obj->data.ptr = std::malloc(sizeof(LinkedList));
	new (obj->data.ptr) LinkedList();
	return obj;
}

constexpr void ListObjLPush(RedisObj* obj, Sds* value)
{

}

constexpr void ListObjRPush(RedisObj* obj, Sds* value)
{
}

constexpr Sds* ListObjLPop(RedisObj* obj)
{
	return nullptr;
}

constexpr Sds* ListObjRPop(RedisObj* obj)
{
	return nullptr;
}