#pragma once

#include <algorithm>

#include "redisobj.h"
#include "../DataStruct/linkedlist.h"

inline RedisObj* ListObjectCreate()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_LIST;
	obj->encoding = ObjEncoding::REDIS_ENCODING_LINKEDLIST;
	obj->data.ptr = new LinkedList;
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline void ListObjectLPush(RedisObj* obj, Sds* value)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	list.push_front(Sds::create(value));
}

inline void ListObjectRPush(RedisObj* obj, Sds* value)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	list.push_back(Sds::create(value));
}

inline unique_ptr<ValueRef> ListObjectLPop(RedisObj* obj)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	if (list.size() > 0)
	{
		Sds* value = reinterpret_cast<Sds*>(list.front());
		list.pop_front();
		return make_unique<ValueRef>(value, nullptr);
	}
	return nullptr;
}

inline unique_ptr<ValueRef> ListObjectRPop(RedisObj* obj)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	if (list.size() > 0)
	{
		Sds* value = reinterpret_cast<Sds*>(list.back());
		list.pop_back();
		return make_unique<ValueRef>(value, nullptr);
	}
	return nullptr;
}

inline vector<unique_ptr<ValueRef>> ListObjectRange(RedisObj* obj, int start, int end)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;
	int size = list.size();

	start = (start % size + size) % size;
	end = (end % size + size) % size;
	
	auto startit = list.begin();
	auto endit = list.begin();
	std::advance(startit, start);
	std::advance(endit, end + 1);

	for (auto it = startit; it != endit; it++)
		result.emplace_back(make_unique<ValueRef>(reinterpret_cast<Sds*>(*it), obj));

	return result;
}

inline void ListObjectDestroy(RedisObj* obj)
{
	LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
	for (auto& value : list)
	{
		Sds::destroy(reinterpret_cast<Sds*>(value));
	}
	delete& list;
	std::free(obj);
}