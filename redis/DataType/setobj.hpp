#pragma once

#include "redisobj.h"
#include "../DataStruct/hashtable.h"

using std::vector;
using std::unique_ptr;
using std::make_unique;

inline RedisObj* CreateSetObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_SET;
	obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
	obj->data.ptr = std::malloc(sizeof(HashTable<Sds*>));
	new (obj->data.ptr) HashTable<Sds*>();
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline void SetObjAdd(RedisObj* obj, Sds* member)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	ht[Sds::create(member)] = nullptr;
}

inline void SetObjRemove(RedisObj* obj, Sds* member)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	if (ht.contains(member))
	{
		ht.erase(member);
		Sds::destroy(member);
	}
}

inline auto SetObjMembers(RedisObj* obj)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;
	for (auto& [key, _] : ht)
	{
		result.emplace_back(make_unique<ValueRef>(key, obj));
	}
	return result;
}

inline bool SetObjIsMember(RedisObj* obj, Sds* member)
{
	HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
	return ht.contains(member);
}
