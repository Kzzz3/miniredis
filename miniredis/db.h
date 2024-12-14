#pragma once
#include <list>

#include "DataStruct/hashtable.h"

class RedisDb
{
public:
    HashTable<RedisObj*> kvstore;
    std::list<RedisObj*> deadobj;
};