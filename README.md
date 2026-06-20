# MiniRedis

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.28+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**MiniRedis** 是一个用 C++23 实现的高性能 Redis 服务器，提供了 Redis 的核心功能，包括多种数据结构、持久化、内存管理、事务支持等。

## ✨ 特性

### 数据结构
- **String**: SDS (Simple Dynamic String) 实现，支持多种编码优化
- **List**: 链表实现，支持双端操作
- **Hash**: 哈希表实现，支持字段级操作
- **Set**: 集合实现，基于哈希表
- **Sorted Set**: 有序集合实现，基于红黑树

### 命令支持 (52+ 命令)

#### String 命令
| 命令 | 描述 |
|------|------|
| `SET key value` | 设置 key 的值 |
| `GET key` | 获取 key 的值 |
| `MSET key1 value1 [key2 value2 ...]` | 批量设置多个 key |
| `MGET key1 [key2 ...]` | 批量获取多个 key |
| `INCR key` | 将 key 的值加 1 |
| `DECR key` | 将 key 的值减 1 |
| `APPEND key value` | 追加值到 key |
| `STRLEN key` | 获取 key 值的长度 |
| `SETNX key value` | 仅当 key 不存在时设置 |
| `SETEX key seconds value` | 设置 key 并指定过期时间 |

#### Hash 命令
| 命令 | 描述 |
|------|------|
| `HSET key field value` | 设置哈希表字段 |
| `HGET key field` | 获取哈希表字段 |
| `HDEL key field [field ...]` | 删除哈希表字段 |
| `HKEYS key` | 获取所有字段名 |
| `HGETALL key` | 获取所有字段和值 |
| `HLEN key` | 获取字段数量 |
| `HEXISTS key field` | 检查字段是否存在 |
| `HINCRBY key field increment` | 对字段值进行增量操作 |

#### List 命令
| 命令 | 描述 |
|------|------|
| `LPUSH key value [value ...]` | 左端推入 |
| `RPUSH key value [value ...]` | 右端推入 |
| `LPOP key` | 左端弹出 |
| `RPOP key` | 右端弹出 |
| `LRANGE key start stop` | 获取范围内的元素 |
| `LLEN key` | 获取列表长度 |
| `LINDEX key index` | 获取指定索引的元素 |

#### Set 命令
| 命令 | 描述 |
|------|------|
| `SADD key member [member ...]` | 添加成员 |
| `SREM key member [member ...]` | 删除成员 |
| `SPOP key` | 随机弹出成员 |
| `SMEMBERS key` | 获取所有成员 |
| `SISMEMBER key member` | 检查成员是否存在 |
| `SCARD key` | 获取成员数量 |

#### Sorted Set 命令
| 命令 | 描述 |
|------|------|
| `ZADD key score member [score member ...]` | 添加成员 |
| `ZREM key member [member ...]` | 删除成员 |
| `ZRANGE key start stop` | 获取范围内的成员 |
| `ZREVRANGE key start stop` | 获取范围内的成员（逆序） |
| `ZPOPMIN key` | 弹出分数最小的成员 |
| `ZCARD key` | 获取成员数量 |
| `ZSCORE key member` | 获取成员的分数 |

#### 通用命令
| 命令 | 描述 |
|------|------|
| `DEL key [key ...]` | 删除 key |
| `EXISTS key [key ...]` | 检查 key 是否存在 |
| `TYPE key` | 获取 key 的类型 |
| `TTL key` | 获取 key 的剩余过期时间 |
| `EXPIRE key seconds` | 设置 key 的过期时间 |
| `PING` | 测试连接 |
| `FLUSHALL` | 清空所有数据库 |
| `KEYS pattern` | 获取匹配的 key |

#### 事务命令
| 命令 | 描述 |
|------|------|
| `MULTI` | 开始事务 |
| `EXEC` | 执行事务 |
| `DISCARD` | 取消事务 |
| `WATCH key [key ...]` | 监视 key（乐观锁） |
| `UNWATCH` | 取消监视 |

### 持久化
- **RDB**: 定时快照，支持压缩
- **AOF**: 追加写入，支持重写

### 内存管理
- **LRU 淘汰**: 最近最少使用优先淘汰
- **过期机制**: 惰性删除 + 定期删除

### 网络
- **异步 I/O**: 基于 Asio 的协程网络模型
- **连接管理**: 支持多客户端并发连接

## 🚀 快速开始

### 环境要求

- C++23 编译器 (MSVC 2022+, GCC 13+, Clang 17+)
- CMake 3.28+
- vcpkg (推荐)

### 构建步骤

```bash
# 1. 克隆仓库
git clone https://github.com/yourusername/miniredis.git
cd miniredis

# 2. 安装依赖 (使用 vcpkg)
vcpkg install

# 3. 配置
cmake --preset default

# 4. 构建
cmake --build --preset default --config Release

# 5. 运行
./build/miniredis/Release/miniredis.exe
```

### 使用 CMake Presets

项目支持 CMake Presets，简化构建流程：

```bash
# 配置 (Debug)
cmake --preset debug

# 配置 (Release)
cmake --preset release

# 构建
cmake --build --preset debug
# 或
cmake --build --preset release
```

### 配置 vcpkg

创建 `CMakeUserPresets.json` 配置本地 vcpkg 路径：

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "user-default",
      "inherits": "default",
      "environment": {
        "VCPKG_ROOT": "D:\\vcpkg"
      },
      "cacheVariables": {
        "VCPKG_TARGET_TRIPLET": "x64-windows"
      }
    }
  ]
}
```

## 📖 使用示例

### 启动服务器

```bash
./build/miniredis/Release/miniredis.exe
```

服务器默认监听 `127.0.0.1:10087`。

### 使用 redis-cli 连接

```bash
redis-cli -p 10087
```

### 基本操作

```bash
# String 操作
127.0.0.1:10087> SET mykey "Hello World"
OK
127.0.0.1:10087> GET mykey
"Hello World"
127.0.0.1:10087> INCR counter
(integer) 1
127.0.0.1:10087> INCR counter
(integer) 2

# Hash 操作
127.0.0.1:10087> HSET user:1 name "Alice" age 30
(integer) 2
127.0.0.1:10087> HGET user:1 name
"Alice"
127.0.0.1:10087> HGETALL user:1
1) "name"
2) "Alice"
3) "age"
4) "30"

# List 操作
127.0.0.1:10087> LPUSH mylist "world"
(integer) 1
127.0.0.1:10087> LPUSH mylist "hello"
(integer) 2
127.0.0.1:10087> LRANGE mylist 0 -1
1) "hello"
2) "world"

# Set 操作
127.0.0.1:10087> SADD myset "apple" "banana" "cherry"
(integer) 3
127.0.0.1:10087> SMEMBERS myset
1) "apple"
2) "banana"
3) "cherry"
127.0.0.1:10087> SISMEMBER myset "apple"
(integer) 1

# Sorted Set 操作
127.0.0.1:10087> ZADD leaderboard 100 "Alice" 85 "Bob" 92 "Charlie"
(integer) 3
127.0.0.1:10087> ZRANGE leaderboard 0 -1 WITHSCORES
1) "Bob"
2) "85"
3) "Charlie"
4) "92"
5) "Alice"
6) "100"

# 事务操作
127.0.0.1:10087> MULTI
OK
127.0.0.1:10087> SET key1 "value1"
QUEUED
127.0.0.1:10087> SET key2 "value2"
QUEUED
127.0.0.1:10087> EXEC
1) OK
2) OK

# 过期操作
127.0.0.1:10087> SET mykey "value"
OK
127.0.0.1:10087> EXPIRE mykey 10
(integer) 1
127.0.0.1:10087> TTL mykey
(integer) 8
```

## 🏗️ 架构设计

### 项目结构

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
│   ├── DataStructTest/     # 数据结构测试
│   │   ├── main.cpp
│   │   ├── sds_test.hpp
│   │   ├── intset_test.hpp
│   │   ├── ziplist_test.hpp
│   │   └── zset_test.hpp
│   └── DataTypeTest/       # 数据类型测试
│       ├── main.cpp
│       ├── stringcmd_test.hpp
│       ├── hashcmd_test.hpp
│       ├── listcmd_test.hpp
│       ├── setcmd_test.hpp
│       ├── zsetcmd_test.hpp
│       ├── expirecmd_test.hpp
│       ├── generalcmd_test.hpp
│       └── transactioncmd_test.hpp
└── benchmark/              # 性能测试
```

### 核心组件

#### 1. 服务器 (Server)
- 基于 Asio 的异步网络模型
- 协程处理客户端连接
- 命令分发和执行

#### 2. 命令系统 (Command)
- 命令注册和查找
- 参数验证
- 事务支持

#### 3. 数据结构 (DataStruct)
- **SDS**: 二进制安全的动态字符串
- **Dict**: 高性能字典，支持渐进式 rehash
- **Ziplist**: 压缩列表，节省内存
- **IntSet**: 整数集合，优化整数存储
- **LinkedList**: 双向链表
- **RBTree**: 红黑树，用于有序集合

#### 4. 数据类型 (DataType)
- 对象系统，支持多种编码
- 引用计数内存管理
- LRU 时间戳

#### 5. 数据库 (Database)
- 多数据库支持
- 过期 key 管理
- 内存淘汰策略

#### 6. 持久化 (Persistence)
- **RDB**: 定时快照，支持压缩
- **AOF**: 追加写入，支持重写

## 🧪 测试

### 运行测试

```bash
# 运行数据结构测试
./build/tests/DataStructTest/Debug/DataStructTest.exe

# 运行数据类型测试 (需要服务器运行)
./build/tests/DataTypeTest/Debug/DataTypeTest.exe

# 使用 CTest
ctest --preset default
```

### 测试覆盖

- **DataStructTest**: 49 个测试
  - SDS: 12 个测试
  - IntSet: 10 个测试
  - Ziplist: 13 个测试
  - ZSet: 14 个测试

- **DataTypeTest**: 53+ 个测试
  - String 命令: 8 个测试
  - Hash 命令: 5 个测试
  - List 命令: 5 个测试
  - Set 命令: 6 个测试
  - ZSet 命令: 6 个测试
  - 通用命令: 14 个测试
  - 过期命令: 7 个测试
  - 事务命令: 9 个测试

## 📊 性能

### 内存优化

- **SDS**: 预分配策略，减少 realloc 次数
- **Ziplist**: 压缩存储，节省内存
- **IntSet**: 整数集合优化
- **Dict**: 渐进式 rehash，避免一次性 rehash 造成的延迟

### 网络优化

- **异步 I/O**: 基于 Asio 的协程模型
- **连接池**: 复用连接，减少创建开销
- **命令管道**: 批量处理命令

## 🔧 配置

### 服务器配置

在 `server.cpp` 中可以修改以下配置：

```cpp
size_t DATABASE_NUM = 16;           // 数据库数量
size_t RDB_TIMER_INTERVAL = 60;     // RDB 定时间隔（秒）
size_t AOF_TIMER_INTERVAL = 60;     // AOF 定时间隔（秒）
size_t DEL_TIMER_INTERVAL = 60;     // 删除定时间隔（秒）
size_t MAXMEMORY = 0;               // 最大内存（0 = 无限制）
int MAXMEMORY_POLICY = 1;           // 内存淘汰策略 (0=noeviction, 1=allkeys-lru, 2=volatile-lru)
```

### 持久化配置

```cpp
bool RDB_ENABLED = false;           // 启用 RDB
bool AOF_ENABLED = false;           // 启用 AOF
```

## 🤝 贡献

欢迎贡献！请遵循以下步骤：

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 代码规范

- 使用 C++23 特性
- 遵循 Google C++ Style Guide
- 添加必要的注释
- 编写单元测试

## 📝 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 🙏 致谢

- [Redis](https://redis.io/) - 原始实现参考
- [Asio](https://think-async.com/Asio/) - 异步 I/O 库
- [yalantinglibs](https://github.com/alibaba/yalantinglibs) - 序列化库
- [Google Test](https://github.com/google/googletest) - 测试框架

## 📧 联系方式

- 项目链接: https://github.com/yourusername/miniredis
- 邮箱: your.email@example.com

---

**MiniRedis** - 用 C++23 实现的高性能 Redis 服务器 🚀
