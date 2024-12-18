#pragma once

#include <memory>

#include "redisobj.h"
#include "Utility/utility.hpp"
#include "DataStruct/hashtable.h"

using std::make_unique;
using std::unique_ptr;
using std::vector;

inline RedisObj* HashObjectCreate()
{
    RedisObj* obj = Allocator::create<RedisObj>();
    obj->type = ObjType::REDIS_HASH;
    obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
    obj->data.ptr = Allocator::create<HashTable<Sds*>>();
    obj->lru = GetSecTimestamp();
    obj->refcount = 1;
    return obj;
}

inline void HashObjectInsert(RedisObj* obj, Sds* field, Sds* value)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    ht[Sds::create(field)] = Sds::create(value);
}

inline auto HashObjectGet(RedisObj* obj, Sds* key)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    return ht.contains(key) ? make_unique<ValueRef>(ht[key], obj) : nullptr;
}

inline void HashObjectDestroy(RedisObj* obj)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    for (auto& [key, value] : ht)
    {
        Sds::destroy(key);
        Sds::destroy(value);
    }
    Allocator::destroy(&ht);
    Allocator::destroy(obj);
}

inline void HashObjectRemove(RedisObj* obj, Sds* filed)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    if (ht.contains(filed))
    {
        auto entry = ht.find(filed);
        Sds* entrykey = entry->first;
        Sds* entryvalue = entry->second;

        ht.erase(filed);

        Sds::destroy(entrykey);
        Sds::destroy(entryvalue);
    }
}

inline auto HashObjectKeys(RedisObj* obj)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    vector<unique_ptr<ValueRef>> result;
    for (auto& [key, _] : ht)
    {
        result.emplace_back(make_unique<ValueRef>(key, obj));
    }
    return result;
}

inline auto HashObjectKVs(RedisObj* obj)
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

inline void HashObjectDataSerialize(ofstream& ofs, RedisObj* obj)
{
    HashTable<Sds*>::serialize_to(ofs, reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr),
                                  [](ofstream& ofs, Sds* value) { Sds::serialize_to(ofs, value); });
}

inline void HashObjectDataDeserialize(ifstream& ifs, RedisObj* obj)
{
    auto func = [](ifstream& ifs) { return Sds::deserialize_from(ifs); };
    obj->data.ptr = HashTable<Sds*>::deserialize_from(ifs, func);
}