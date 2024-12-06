#pragma once
#include "DataStruct/hashtable.h"

class RedisDb
{
public:
	HashTable<RedisObj*> kvstore;
};