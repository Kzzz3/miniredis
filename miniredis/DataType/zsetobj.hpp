#pragma once
#include <ranges>

#include "redisobj.h"
#include "Utility/utility.hpp"
#include "DataStruct/rbtree.h"

inline RedisObj* ZsetObjectCreate()
{
	RedisObj* obj = Allocator::create<RedisObj>();
	obj->type = ObjType::REDIS_ZSET;
	obj->encoding = ObjEncoding::REDIS_ENCODING_RBTREE;
	obj->data.ptr = Allocator::create<RBTree>();
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline void ZsetObjectAdd(RedisObj* obj, double score, Sds* member)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	zset.add(score, Sds::create(member));
}

inline void ZsetObjectRemove(RedisObj* obj, Sds* member)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	if (zset.contains(member))
	{
		auto entry = zset.find(member);
		Sds* entryvalue = entry->second;

		zset.remove(member);
		Sds::destroy(entryvalue);
	}
}

inline void ZsetObjectDestroy(RedisObj* obj)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	if (zset.rbt.size() != 0)
	{
		auto first = zset.getByRank(0);
		auto last = zset.getByRank(-1);

		auto entrys = zset.rangeByScore(first->first, last->first);
		for (auto& [score, member] : entrys)
		{
			Sds::destroy(member);
		}
	}
	Allocator::destroy(&zset);
	Allocator::destroy(obj);
}

inline auto ZsetObjectRange(RedisObj* obj, double minScore, double maxScore)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;

	auto members = zset.rangeByScore(minScore, maxScore);
	for (auto& [score, member] : members)
	{
		result.emplace_back(make_unique<ValueRef>(num2sds(score), nullptr));
		result.emplace_back(make_unique<ValueRef>(member, obj));
	}
	return result;
}

inline auto ZsetObjectRevRange(RedisObj* obj, double minScore, double maxScore)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	vector<unique_ptr<ValueRef>> result;

	auto members = zset.rangeByScore(minScore, maxScore);
	for (auto& [score, member] : std::ranges::reverse_view(members))
	{
		result.emplace_back(make_unique<ValueRef>(num2sds(score), nullptr));
		result.emplace_back(make_unique<ValueRef>(member, obj));
	}
	return result;
}