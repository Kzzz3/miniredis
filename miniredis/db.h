#pragma once
#include <asio.hpp>

#include <list>
#include <vector>
#include <optional>

#include "DataType/redisobj.h"
#include "DataType/setobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/zsetobj.hpp"
#include "DataType/hashobj.hpp"
#include "DataType/stringobj.hpp"
#include "DataStruct/hashtable.h"

using std::list;
using std::vector;
using std::optional;
using asio::detached;
using asio::co_spawn;
using asio::awaitable;
using asio::io_context;
using asio::thread_pool;
using asio::steady_timer;
using asio::use_awaitable;

extern uint32_t DATABASE_NUM;
constexpr uint64_t RDB_TIMER_INTERVAL = 60;
constexpr uint64_t DEL_TIMER_INTERVAL = 60;

class RedisDb
{
public:
    RedisDb(thread_pool& work_executor, io_context& io_context);
    ~RedisDb();

    awaitable<void> rdbTimerHandler();
    awaitable<void> delObjectHandler();

    HashTable<RedisObj*>& getKVStore(Sds* key);
    HashTable<RedisObj*>& getExpiredKVStore(Sds* key);

    void destroyRedisObj(RedisObj* obj);

    // rdb
    void loadRDB(const string& path);
    void storeRDB(const string& path);
    void serialize_to(ofstream& ofs);
    void deserialize_from(ifstream& ifs);
    static RedisObj* valueDeserializeFunc(ifstream& ifs);
    static void valueSerializeFunc(ofstream& ofs, RedisObj* obj);

public:
    // data
    list<RedisObj*> deadobj;
    vector<HashTable<RedisObj*>> kvstores;
    vector<HashTable<RedisObj*>> expired_kvstores;

    // timer
    asio::steady_timer rdb_timer;
    asio::steady_timer delobj_timer;
};