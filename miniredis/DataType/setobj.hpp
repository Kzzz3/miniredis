#pragma once
#include "redisobj.h"
#include "../DataStruct/hashtable.h"

using std::vector;
using std::unique_ptr;
using std::make_unique;

inline RedisObj* SetObjectCreate()
{
    RedisObj* obj = Allocator::create<RedisObj>();
    obj->type = ObjType::REDIS_SET;
    obj->encoding = ObjEncoding::REDIS_ENCODING_HT;
    obj->data.ptr = Allocator::create<HashTable<Sds*>>();
    obj->lru = GetSecTimestamp();
    obj->refcount = 1;
    return obj;
}

inline void SetObjectAdd(RedisObj* obj, Sds* member)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    ht[Sds::create(member)] = nullptr;
}

inline void SetObjectRemove(RedisObj* obj, Sds* member)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    if (ht.contains(member))
    {
        auto entry = ht.find(member);
        Sds* entrykey = entry->first;

        ht.erase(member);

        Sds::destroy(entrykey);
    }
}

inline void SetObjectDestroy(RedisObj* obj)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    for (auto& [key, _] : ht)
    {
        Sds::destroy(key);
    }
    Allocator::destroy(&ht);
    Allocator::destroy(obj);
}

inline auto SetObjectMembers(RedisObj* obj)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    vector<unique_ptr<ValueRef>> result;
    for (auto& [key, _] : ht)
    {
        result.emplace_back(make_unique<ValueRef>(key, obj));
    }
    return result;
}

inline bool SetObjectIsMember(RedisObj* obj, Sds* member)
{
    HashTable<Sds*>& ht = *reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr);
    return ht.contains(member);
}

inline void SetObjectDataSerialize(ofstream& ofs, RedisObj* obj)
{
    HashTable<Sds*>::serialize_to(ofs, reinterpret_cast<HashTable<Sds*>*>(obj->data.ptr),
                                  [](ofstream& ofs, Sds* value) {});
}

inline void SetObjectDataDeserialize(ifstream& ifs, RedisObj* obj)
{
    obj->data.ptr = HashTable<Sds*>::deserialize_from(ifs, [](ifstream& ifs) { return nullptr; });
}