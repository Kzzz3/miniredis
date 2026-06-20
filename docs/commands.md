# 命令参考

MiniRedis 支持 52+ 个 Redis 命令，覆盖所有主要数据类型。

## String 命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `SET key value` | 设置 key 的值 | `SET mykey "Hello"` |
| `GET key` | 获取 key 的值 | `GET mykey` |
| `MSET key1 value1 [key2 value2 ...]` | 批量设置多个 key | `MSET k1 v1 k2 v2` |
| `MGET key1 [key2 ...]` | 批量获取多个 key | `MGET k1 k2` |
| `INCR key` | 将 key 的值加 1 | `INCR counter` |
| `DECR key` | 将 key 的值减 1 | `DECR counter` |
| `APPEND key value` | 追加值到 key | `APPEND mykey " World"` |
| `STRLEN key` | 获取 key 值的长度 | `STRLEN mykey` |
| `SETNX key value` | 仅当 key 不存在时设置 | `SETNX mykey "value"` |
| `SETEX key seconds value` | 设置 key 并指定过期时间 | `SETEX mykey 10 "value"` |

### 示例

```bash
# 基本操作
127.0.0.1:10087> SET mykey "Hello World"
OK
127.0.0.1:10087> GET mykey
"Hello World"

# 批量操作
127.0.0.1:10087> MSET k1 "v1" k2 "v2" k3 "v3"
OK
127.0.0.1:10087> MGET k1 k2 k3
1) "v1"
2) "v2"
3) "v3"

# 计数器
127.0.0.1:10087> SET counter 0
OK
127.0.0.1:10087> INCR counter
(integer) 1
127.0.0.1:10087> INCR counter
(integer) 2
127.0.0.1:10087> DECR counter
(integer) 1

# 追加
127.0.0.1:10087> SET mykey "Hello"
OK
127.0.0.1:10087> APPEND mykey " World"
(integer) 11
127.0.0.1:10087> GET mykey
"Hello World"

# 条件设置
127.0.0.1:10087> SETNX mykey "value"
(integer) 1  # 成功（key 不存在）
127.0.0.1:10087> SETNX mykey "newvalue"
(integer) 0  # 失败（key 已存在）
```

## Hash 命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `HSET key field value` | 设置哈希表字段 | `HSET user:1 name "Alice"` |
| `HGET key field` | 获取哈希表字段 | `HGET user:1 name` |
| `HDEL key field [field ...]` | 删除哈希表字段 | `HDEL user:1 name` |
| `HKEYS key` | 获取所有字段名 | `HKEYS user:1` |
| `HGETALL key` | 获取所有字段和值 | `HGETALL user:1` |
| `HLEN key` | 获取字段数量 | `HLEN user:1` |
| `HEXISTS key field` | 检查字段是否存在 | `HEXISTS user:1 name` |
| `HINCRBY key field increment` | 对字段值进行增量操作 | `HINCRBY user:1 age 1` |

### 示例

```bash
# 设置多个字段
127.0.0.1:10087> HSET user:1 name "Alice" age 30 city "Beijing"
(integer) 3

# 获取单个字段
127.0.0.1:10087> HGET user:1 name
"Alice"

# 获取所有字段和值
127.0.0.1:10087> HGETALL user:1
1) "name"
2) "Alice"
3) "age"
4) "30"
5) "city"
6) "Beijing"

# 检查字段是否存在
127.0.0.1:10087> HEXISTS user:1 name
(integer) 1
127.0.0.1:10087> HEXISTS user:1 email
(integer) 0

# 增量操作
127.0.0.1:10087> HINCRBY user:1 age 5
(integer) 35
```

## List 命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `LPUSH key value [value ...]` | 左端推入 | `LPUSH mylist "world"` |
| `RPUSH key value [value ...]` | 右端推入 | `RPUSH mylist "hello"` |
| `LPOP key` | 左端弹出 | `LPOP mylist` |
| `RPOP key` | 右端弹出 | `RPOP mylist` |
| `LRANGE key start stop` | 获取范围内的元素 | `LRANGE mylist 0 -1` |
| `LLEN key` | 获取列表长度 | `LLEN mylist` |
| `LINDEX key index` | 获取指定索引的元素 | `LINDEX mylist 0` |

### 示例

```bash
# 推入元素
127.0.0.1:10087> LPUSH mylist "c" "b" "a"
(integer) 3
127.0.0.1:10087> RPUSH mylist "d" "e"
(integer) 5

# 获取所有元素
127.0.0.1:10087> LRANGE mylist 0 -1
1) "a"
2) "b"
3) "c"
4) "d"
5) "e"

# 弹出元素
127.0.0.1:10087> LPOP mylist
"a"
127.0.0.1:10087> RPOP mylist
"e"

# 获取长度
127.0.0.1:10087> LLEN mylist
(integer) 3

# 获取指定索引
127.0.0.1:10087> LINDEX mylist 0
"b"
127.0.0.1:10087> LINDEX mylist -1
"d"
```

## Set 命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `SADD key member [member ...]` | 添加成员 | `SADD myset "apple"` |
| `SREM key member [member ...]` | 删除成员 | `SREM myset "apple"` |
| `SPOP key` | 随机弹出成员 | `SPOP myset` |
| `SMEMBERS key` | 获取所有成员 | `SMEMBERS myset` |
| `SISMEMBER key member` | 检查成员是否存在 | `SISMEMBER myset "apple"` |
| `SCARD key` | 获取成员数量 | `SCARD myset` |

### 示例

```bash
# 添加成员
127.0.0.1:10087> SADD myset "apple" "banana" "cherry"
(integer) 3

# 获取所有成员
127.0.0.1:10087> SMEMBERS myset
1) "apple"
2) "banana"
3) "cherry"

# 检查成员是否存在
127.0.0.1:10087> SISMEMBER myset "apple"
(integer) 1
127.0.0.1:10087> SISMEMBER myset "grape"
(integer) 0

# 获取成员数量
127.0.0.1:10087> SCARD myset
(integer) 3

# 删除成员
127.0.0.1:10087> SREM myset "apple"
(integer) 1
127.0.0.1:10087> SMEMBERS myset
1) "banana"
2) "cherry"

# 随机弹出
127.0.0.1:10087> SPOP myset
"banana"
```

## Sorted Set 命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `ZADD key score member [score member ...]` | 添加成员 | `ZADD myzset 100 "Alice"` |
| `ZREM key member [member ...]` | 删除成员 | `ZREM myzset "Alice"` |
| `ZRANGE key start stop` | 获取范围内的成员 | `ZRANGE myzset 0 -1` |
| `ZREVRANGE key start stop` | 获取范围内的成员（逆序） | `ZREVRANGE myzset 0 -1` |
| `ZPOPMIN key` | 弹出分数最小的成员 | `ZPOPMIN myzset` |
| `ZCARD key` | 获取成员数量 | `ZCARD myzset` |
| `ZSCORE key member` | 获取成员的分数 | `ZSCORE myzset "Alice"` |

### 示例

```bash
# 添加成员
127.0.0.1:10087> ZADD leaderboard 100 "Alice" 85 "Bob" 92 "Charlie"
(integer) 3

# 获取所有成员（按分数排序）
127.0.0.1:10087> ZRANGE leaderboard 0 -1 WITHSCORES
1) "Bob"
2) "85"
3) "Charlie"
4) "92"
5) "Alice"
6) "100"

# 获取所有成员（按分数逆序）
127.0.0.1:10087> ZREVRANGE leaderboard 0 -1 WITHSCORES
1) "Alice"
2) "100"
3) "Charlie"
4) "92"
5) "Bob"
6) "85"

# 获取成员分数
127.0.0.1:10087> ZSCORE leaderboard "Alice"
"100"

# 获取成员数量
127.0.0.1:10087> ZCARD leaderboard
(integer) 3

# 弹出分数最小的成员
127.0.0.1:10087> ZPOPMIN leaderboard
1) "Bob"
2) "85"
```

## 通用命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `DEL key [key ...]` | 删除 key | `DEL mykey` |
| `EXISTS key [key ...]` | 检查 key 是否存在 | `EXISTS mykey` |
| `TYPE key` | 获取 key 的类型 | `TYPE mykey` |
| `TTL key` | 获取 key 的剩余过期时间 | `TTL mykey` |
| `EXPIRE key seconds` | 设置 key 的过期时间 | `EXPIRE mykey 10` |
| `PING` | 测试连接 | `PING` |
| `FLUSHALL` | 清空所有数据库 | `FLUSHALL` |

### 示例

```bash
# 检查 key 是否存在
127.0.0.1:10087> SET mykey "value"
OK
127.0.0.1:10087> EXISTS mykey
(integer) 1
127.0.0.1:10087> EXISTS nonexistent
(integer) 0

# 获取 key 类型
127.0.0.1:10087> SET stringkey "value"
OK
127.0.0.1:10087> TYPE stringkey
"string"
127.0.0.1:10087> HSET hashkey field value
OK
127.0.0.1:10087> TYPE hashkey
"hash"

# 设置过期时间
127.0.0.1:10087> SET mykey "value"
OK
127.0.0.1:10087> EXPIRE mykey 10
(integer) 1
127.0.0.1:10087> TTL mykey
(integer) 8
127.0.0.1:10087> TTL mykey
(integer) 5

# 删除 key
127.0.0.1:10087> DEL mykey
(integer) 1
127.0.0.1:10087> EXISTS mykey
(integer) 0
```

## 事务命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `MULTI` | 开始事务 | `MULTI` |
| `EXEC` | 执行事务 | `EXEC` |
| `DISCARD` | 取消事务 | `DISCARD` |
| `WATCH key [key ...]` | 监视 key（乐观锁） | `WATCH mykey` |
| `UNWATCH` | 取消监视 | `UNWATCH` |

### 示例

```bash
# 基本事务
127.0.0.1:10087> MULTI
OK
127.0.0.1:10087> SET key1 "value1"
QUEUED
127.0.0.1:10087> SET key2 "value2"
QUEUED
127.0.0.1:10087> EXEC
1) OK
2) OK

# 取消事务
127.0.0.1:10087> MULTI
OK
127.0.0.1:10087> SET key1 "value1"
QUEUED
127.0.0.1:10087> DISCARD
OK

# 乐观锁
127.0.0.1:10087> SET mykey "value"
OK
127.0.0.1:10087> WATCH mykey
OK
127.0.0.1:10087> MULTI
OK
127.0.0.1:10087> SET mykey "newvalue"
QUEUED
127.0.0.1:10087> EXEC
1) OK
```
