#pragma once

#include <memory>

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/hashtable.h"

using std::malloc;
using std::vector;
using std::unique_ptr;
using std::make_unique;

inline RedisObj* CreateHashObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_HASH;
	obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
	obj->data.ptr = std::malloc(sizeof(HashTable<Sds*>));
	new (obj->data.ptr) HashTable<Sds*>();
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline void HashObjInsert(RedisObj* obj, Sds* key, Sds* value)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	ht[key] = Sds::create(value);
}

inline auto HashObjGet(RedisObj* obj, Sds* key)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	return ht.contains(key) ? make_unique<ValueRef>(ht[key], obj) : nullptr;
}

inline void HashObjRemove(RedisObj* obj, Sds* key)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	if (ht.contains(key))
	{
		Sds::destroy(ht[key]);
		ht.erase(key);
	}
}

inline auto HashObjKeys(RedisObj* obj)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;
	for (auto& [key, _] : ht)
	{
		result.emplace_back(make_unique<ValueRef>(key, obj));
	}
	return result;
}

inline auto HashObjKVs(RedisObj* obj)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;
	for (auto& [key, value] : ht)
	{
		result.emplace_back(make_unique<ValueRef>(key, obj));
		result.emplace_back(make_unique<ValueRef>(value, obj));
	}
	return result;
}