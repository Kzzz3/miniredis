#pragma once
#include <ranges>

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/rbtree.h"

inline RedisObj* CreateZsetObject()
{
	RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_ZSET;
	obj->encoding = ObjEncoding::REDIS_ENCODING_RBTREE;
	obj->data.ptr = std::malloc(sizeof(RBTree));
	new (obj->data.ptr) RBTree();
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline void ZsetObjAdd(RedisObj* obj, double score, Sds* member)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	zset.add(score, Sds::create(member));
}

inline void ZsetObjRemove(RedisObj* obj, Sds* member)
{
	RBTree& zset = *reinterpret_cast<RBTree*>(obj->data.ptr);
	if (zset.contains(member))
	{
		zset.remove(member);
		Sds::destroy(member);
	}
}

inline auto ZsetObjRange(RedisObj* obj, double minScore, double maxScore)
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

inline auto ZsetObjRevRange(RedisObj* obj, double minScore, double maxScore)
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
