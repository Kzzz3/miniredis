#pragma once
#include <list>

#include "DataType/redisobj.h"
#include "DataType/setobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/zsetobj.hpp"
#include "DataType/hashobj.hpp"
#include "DataType/stringobj.hpp"
#include "DataStruct/hashtable.h"

class RedisDb
{
public:
    RedisDb();
    ~RedisDb();

public:
    HashTable<RedisObj*>* kvstore;
    std::list<RedisObj*>* deadobj;

    void serialize_to(ofstream& ofs);
    void deserialize_from(ifstream& ifs);
};