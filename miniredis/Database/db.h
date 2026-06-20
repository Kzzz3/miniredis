#pragma once
#include <asio.hpp>

#include <list>
#include <memory>
#include <vector>
#include <cassert>
#include <optional>

#include "aof.h"
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
using std::shared_ptr;
using std::make_shared;
using asio::detached;
using asio::co_spawn;
using asio::awaitable;
using asio::steady_timer;
using asio::use_awaitable;

class Server;

extern Server server;
extern bool RDB_ENABLED;
extern bool AOF_ENABLED;
extern size_t DATABASE_NUM;
extern size_t RDB_TIMER_INTERVAL;
extern size_t DEL_TIMER_INTERVAL;

class RedisDb
{
public:
    RedisDb(asio::thread_pool& work_executor, asio::io_context& io_context);
    ~RedisDb();

    // timer handler
    awaitable<void> rdbTimerHandler();
    awaitable<void> delObjectHandler();

    // db operation
    void loadPersistedData();
    void startDataPersistence();
    HashTable<RedisObj*>& getKVStore(Sds* key);
    HashTable<RedisObj*>& getExpiredKVStore(Sds* key);
    bool isKeyExpired(Sds* key);  // Check and delete if expired
    void deleteExpiredKey(Sds* key);  // Delete expired key
    void evictLRU();  // Evict least recently used keys
    size_t getMemoryUsage() const;  // Get current memory usage

    // rdb
    void startRdb();
    void loadRDB(const string& path);
    void storeRDB(const string& path);
    static RedisObj* valueDeserializeFunc(ifstream& ifs);
    static void valueSerializeFunc(ofstream& ofs, RedisObj* obj);

public:
    asio::io_context& io_context;
    asio::thread_pool& work_executor;

    // aof
    Aof aof;

    // data
    list<RedisObj*> deadobj;
    vector<HashTable<RedisObj*>> kvstores;
    vector<HashTable<RedisObj*>> expired_kvstores;

    // timer
    asio::steady_timer rdb_timer;
    asio::steady_timer delobj_timer;
};