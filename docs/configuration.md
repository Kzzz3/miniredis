# 配置说明

MiniRedis 的配置主要在 `server.cpp` 文件中，可以通过修改源代码来调整服务器行为。

## 服务器配置

### 基本配置

在 `server.cpp` 中可以修改以下配置：

```cpp
// 数据库数量
size_t DATABASE_NUM = 16;

// 定时器间隔（秒）
size_t RDB_TIMER_INTERVAL = 60;     // RDB 定时间隔
size_t AOF_TIMER_INTERVAL = 60;     // AOF 定时间隔
size_t DEL_TIMER_INTERVAL = 60;     // 删除定时间隔

// 内存配置
size_t MAXMEMORY = 0;               // 最大内存（0 = 无限制）
int MAXMEMORY_POLICY = 1;           // 内存淘汰策略
```

### 配置说明

| 配置项 | 类型 | 默认值 | 描述 |
|--------|------|--------|------|
| `DATABASE_NUM` | size_t | 16 | 数据库数量 |
| `RDB_TIMER_INTERVAL` | size_t | 60 | RDB 定时间隔（秒） |
| `AOF_TIMER_INTERVAL` | size_t | 60 | AOF 定时间隔（秒） |
| `DEL_TIMER_INTERVAL` | size_t | 60 | 删除定时间隔（秒） |
| `MAXMEMORY` | size_t | 0 | 最大内存限制（字节） |
| `MAXMEMORY_POLICY` | int | 1 | 内存淘汰策略 |

## 持久化配置

### RDB 配置

```cpp
// 启用 RDB
bool RDB_ENABLED = false;
```

RDB 是定时快照持久化，特点：
- 定时生成数据快照
- 压缩存储
- 恢复速度快
- 可能丢失最后一次快照后的数据

### AOF 配置

```cpp
// 启用 AOF
bool AOF_ENABLED = false;
```

AOF 是追加写入持久化，特点：
- 记录每个写命令
- 数据安全性高
- 文件较大
- 支持重写

### 注意事项

- RDB 和 AOF 不能同时启用（当前版本）
- 修改配置后需要重新编译

## 内存淘汰策略

### 策略类型

| 策略值 | 策略名称 | 描述 |
|--------|----------|------|
| 0 | noeviction | 不淘汰，内存满时返回错误 |
| 1 | allkeys-lru | 从所有 key 中淘汰最近最少使用的 |
| 2 | volatile-lru | 从设置了过期时间的 key 中淘汰最近最少使用的 |

### 配置示例

```cpp
// 设置最大内存为 100MB
size_t MAXMEMORY = 100 * 1024 * 1024;

// 设置淘汰策略为 allkeys-lru
int MAXMEMORY_POLICY = 1;
```

## 网络配置

### 监听地址

服务器默认监听 `127.0.0.1:10087`。

修改监听地址需要修改 `server.cpp` 中的 `startServer` 函数：

```cpp
awaitable<void> Server::startServer(const std::string& host, uint16_t port) {
    // 修改这里的 host 和 port
    tcp::endpoint endpoint(tcp::v4(), port);
    // ...
}
```

### 缓冲区大小

在 `connection.h` 中可以修改缓冲区大小：

```cpp
constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024 * 15;  // 15MB
```

## 编译配置

### CMake 配置

在 `CMakeLists.txt` 中可以修改编译选项：

```cmake
# 编译选项
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 /W3 /WX-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W3 /WX-")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
endif()
```

### 编译选项说明

| 选项 | 描述 |
|------|------|
| `/O2` 或 `-O2` | 优化级别 2 |
| `/W3` 或 `-Wall` | 警告级别 |
| `/WX-` | 不将警告视为错误 |

## 依赖配置

### vcpkg 依赖

在 `vcpkg.json` 中配置依赖：

```json
{
  "name": "miniredis",
  "version": "1.0.0",
  "dependencies": [
    "asio",
    "yalantinglibs",
    "zlib",
    "jemalloc",
    "gtest"
  ]
}
```

### 依赖说明

| 依赖 | 版本 | 描述 |
|------|------|------|
| asio | latest | 异步 I/O 库 |
| yalantinglibs | latest | 序列化库 |
| zlib | latest | 压缩库 |
| jemalloc | latest | 内存分配器 |
| gtest | latest | 测试框架 |

## 运行时配置

### 环境变量

| 环境变量 | 描述 |
|----------|------|
| `VCPKG_ROOT` | vcpkg 安装路径 |
| `VCPKG_DEFAULT_TRIPLET` | 默认 triplet |

### 日志配置

当前版本使用标准输出输出日志，可以通过修改代码来调整日志级别。

## 高级配置

### 自定义数据结构

可以修改 `DataStruct/` 目录下的文件来自定义数据结构实现。

### 自定义命令

可以修改 `Command/` 目录下的文件来添加自定义命令。

### 自定义持久化

可以修改 `Database/` 目录下的文件来自定义持久化策略。

## 配置示例

### 开发环境配置

```cpp
// 开发环境：小内存，频繁持久化
size_t DATABASE_NUM = 4;
size_t RDB_TIMER_INTERVAL = 30;
size_t AOF_TIMER_INTERVAL = 30;
size_t DEL_TIMER_INTERVAL = 30;
size_t MAXMEMORY = 100 * 1024 * 1024;  // 100MB
int MAXMEMORY_POLICY = 1;
bool RDB_ENABLED = true;
bool AOF_ENABLED = false;
```

### 生产环境配置

```cpp
// 生产环境：大内存，定期持久化
size_t DATABASE_NUM = 16;
size_t RDB_TIMER_INTERVAL = 300;
size_t AOF_TIMER_INTERVAL = 300;
size_t DEL_TIMER_INTERVAL = 60;
size_t MAXMEMORY = 1024 * 1024 * 1024;  // 1GB
int MAXMEMORY_POLICY = 1;
bool RDB_ENABLED = true;
bool AOF_ENABLED = false;
```

### 测试环境配置

```cpp
// 测试环境：最小配置
size_t DATABASE_NUM = 1;
size_t RDB_TIMER_INTERVAL = 10;
size_t AOF_TIMER_INTERVAL = 10;
size_t DEL_TIMER_INTERVAL = 10;
size_t MAXMEMORY = 0;  // 无限制
int MAXMEMORY_POLICY = 0;
bool RDB_ENABLED = false;
bool AOF_ENABLED = false;
```
