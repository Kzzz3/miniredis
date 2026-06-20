# 架构设计

MiniRedis 采用模块化设计，主要包含以下组件：

## 项目结构

```
miniredis/
├── CMakeLists.txt          # 根 CMake 配置
├── CMakePresets.json        # CMake Presets 配置
├── vcpkg.json              # vcpkg 依赖配置
├── README.md               # 项目文档
├── miniredis/              # 源代码
│   ├── server.h/cpp        # 服务器核心
│   ├── Command/            # 命令实现
│   │   ├── command.h       # 命令定义和注册
│   │   ├── stringcmd.cpp   # String 命令
│   │   ├── hashcmd.cpp     # Hash 命令
│   │   ├── listcmd.cpp     # List 命令
│   │   ├── setcmd.cpp      # Set 命令
│   │   ├── zsetcmd.cpp     # Sorted Set 命令
│   │   ├── generalcmd.cpp  # 通用命令
│   │   └── transactioncmd.cpp # 事务命令
│   ├── DataStruct/         # 数据结构实现
│   │   ├── sds.h/cpp       # SDS (Simple Dynamic String)
│   │   ├── hashtable.h     # 哈希表
│   │   ├── dict.h          # 高性能字典（渐进式 rehash）
│   │   ├── ziplist.h/cpp   # 压缩列表
│   │   ├── intset.h/cpp    # 整数集合
│   │   ├── linkedlist.h/cpp # 链表
│   │   └── rbtree.h/cpp    # 红黑树
│   ├── DataType/           # 数据类型实现
│   │   ├── redisobj.h      # Redis 对象定义
│   │   ├── stringobj.hpp   # String 对象
│   │   ├── hashobj.hpp     # Hash 对象
│   │   ├── listobj.hpp     # List 对象
│   │   ├── setobj.hpp      # Set 对象
│   │   └── zsetobj.hpp     # Sorted Set 对象
│   ├── Database/           # 数据库实现
│   │   ├── db.h/cpp        # 数据库核心
│   │   ├── rdb.h/cpp       # RDB 持久化
│   │   └── aof.h/cpp       # AOF 持久化
│   ├── Networking/         # 网络层
│   │   ├── connection.h    # 连接管理
│   │   └── server.h/cpp    # 服务器网络
│   └── Utility/            # 工具类
│       ├── allocator.hpp   # 内存分配器
│       └── utility.hpp     # 工具函数
├── tests/                  # 测试代码
└── benchmark/              # 性能测试
```

## 核心组件

### 1. 服务器 (Server)

服务器是系统的核心，负责：
- 接受客户端连接
- 解析 RESP 协议
- 分发命令到相应的处理器
- 管理连接生命周期

```cpp
class Server {
    awaitable<void> acceptConnections();
    awaitable<void> handleConnection(shared_ptr<Connection> conn);
    awaitable<expected<Command, error_code>> readCommandFromClient(shared_ptr<Connection> conn);
    function<bool(shared_ptr<Connection>, Command&)> CommandProcess(Command& cmd);
};
```

### 2. 命令系统 (Command)

命令系统负责：
- 命令注册和查找
- 参数验证
- 事务支持
- 命令执行

```cpp
// 命令注册
inline const unordered_map<string, function<bool(shared_ptr<Connection>, Command&)>> commands_map = {
    {"set", CmdSet},
    {"get", CmdGet},
    // ...
};

// 命令查找
inline function<bool(shared_ptr<Connection>, Command&)> GetCommandHandler(Sds* cmdtype) {
    string command(cmdtype->buf, cmdtype->length());
    auto it = commands_map.find(command);
    return it != commands_map.end() ? it->second : nullptr;
}
```

### 3. 数据结构 (DataStruct)

#### SDS (Simple Dynamic String)

SDS 是二进制安全的动态字符串，特点：
- 预分配策略，减少 realloc 次数
- 支持多种编码（8位、16位、32位、64位）
- 二进制安全

```cpp
class Sds {
    char* buf;
    size_t length();
    size_t capacity();
    Sds* append(const char* str, size_t len);
    Sds* copy(const char* str, size_t len);
};
```

#### Dict (高性能字典)

Dict 是高性能字典，特点：
- 渐进式 rehash，避免一次性 rehash 造成的延迟
- 两个哈希表实现
- 2 的幂次方大小优化取模运算

```cpp
template <typename K, typename V>
class Dict {
    bool insert(const K& key, const V& value);
    V* find(const K& key);
    bool erase(const K& key);
    bool contains(const K& key);
};
```

#### Ziplist (压缩列表)

Ziplist 是压缩列表，特点：
- 节省内存
- 连续内存存储
- 支持多种编码

#### IntSet (整数集合)

IntSet 是整数集合，特点：
- 优化整数存储
- 支持自动升级
- 节省内存

#### LinkedList (双向链表)

LinkedList 是双向链表，特点：
- 双端操作
- O(1) 复杂度的插入和删除

#### RBTree (红黑树)

RBTree 是红黑树，特点：
- 有序存储
- O(log n) 复杂度的查找、插入、删除

### 4. 数据类型 (DataType)

数据类型系统负责：
- 对象管理
- 编码转换
- 内存管理

```cpp
enum class ObjType : uint8_t {
    REDIS_STRING,
    REDIS_LIST,
    REDIS_HASH,
    REDIS_SET,
    REDIS_ZSET
};

enum class ObjEncoding : uint8_t {
    REDIS_ENCODING_INT,
    REDIS_ENCODING_EMBSTR,
    REDIS_ENCODING_RAW,
    REDIS_ENCODING_HT,
    REDIS_ENCODING_LINKEDLIST,
    REDIS_ENCODING_ZIPLIST,
    REDIS_ENCODING_INTSET,
    REDIS_ENCODING_RBTREE,
};
```

### 5. 数据库 (Database)

数据库系统负责：
- 多数据库支持
- 过期 key 管理
- 内存淘汰策略
- 持久化

```cpp
class RedisDb {
    vector<HashTable<RedisObj*>> kvstores;
    vector<HashTable<RedisObj*>> expired_kvstores;
    list<RedisObj*> deadobj;
    
    HashTable<RedisObj*>& getKVStore(Sds* key);
    bool isKeyExpired(Sds* key);
    void evictLRU();
};
```

### 6. 网络层 (Networking)

网络层负责：
- 异步 I/O
- 连接管理
- 协程处理

```cpp
class Connection {
    uint64_t id;
    atomic<ConnectionState> state;
    tcp::socket socket;
    streambuf read_buffer;
    
    // 事务状态
    bool in_transaction;
    vector<Command> command_queue;
    unordered_set<Sds*> watched_keys;
};
```

## 数据流

```
客户端连接
    ↓
服务器接受连接
    ↓
读取命令（RESP 协议）
    ↓
解析命令
    ↓
查找命令处理器
    ↓
执行命令
    ↓
返回结果
```

## 内存管理

### 引用计数

```cpp
class RedisObj {
    uint32_t refcount;
    
    void addRef() { refcount++; }
    void release() {
        refcount--;
        if (refcount == 0) {
            // 销毁对象
        }
    }
};
```

### LRU 淘汰

```cpp
void RedisDb::evictLRU() {
    // 查找最近最少使用的 key
    // 删除该 key
}
```

## 持久化

### RDB (快照)

- 定时生成快照
- 压缩存储
- 恢复速度快

### AOF (追加写入)

- 记录每个写命令
- 支持重写
- 数据安全性高

## 事务

### 实现

```cpp
class Connection {
    bool in_transaction;
    vector<Command> command_queue;
    unordered_set<Sds*> watched_keys;
    
    void multi();
    void exec();
    void discard();
    void watch(Sds* key);
    void unwatch();
};
```

### 流程

1. MULTI 开始事务
2. 命令加入队列
3. EXEC 执行所有命令
4. DISCARD 取消事务
5. WATCH 实现乐观锁
