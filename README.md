# MiniRedis

用 C++23 实现的高性能 Redis 服务器。

## 特性

- **52+ 命令** - 完整的 Redis 核心命令支持
- **5 种数据结构** - String, List, Hash, Set, Sorted Set
- **持久化** - RDB 快照 + AOF 日志
- **内存管理** - LRU 淘汰策略
- **事务** - MULTI/EXEC/DISCARD/WATCH
- **异步 I/O** - 基于 Asio 的协程网络模型

## 快速开始

```bash
# 克隆
git clone https://github.com/kzzz3/miniredis.git
cd miniredis

# 构建
cmake --preset default
cmake --build --preset default --config Release

# 运行
./build/miniredis/Release/miniredis.exe
```

## 使用

```bash
# 连接
redis-cli -p 10087

# 基本操作
127.0.0.1:10087> SET mykey "Hello"
OK
127.0.0.1:10087> GET mykey
"Hello"

# 事务
127.0.0.1:10087> MULTI
OK
127.0.0.1:10087> SET key1 "value1"
QUEUED
127.0.0.1:10087> EXEC
1) OK
```

## 文档

- [命令参考](docs/commands.md) - 所有支持的命令
- [架构设计](docs/architecture.md) - 项目架构说明
- [构建指南](docs/building.md) - 详细构建步骤
- [配置说明](docs/configuration.md) - 服务器配置
- [开发指南](docs/development.md) - 贡献和开发指南

## 测试

```bash
# 运行测试
./build/tests/DataStructTest/Debug/DataStructTest.exe
./build/tests/DataTypeTest/Debug/DataTypeTest.exe

# 使用 CTest
ctest --preset default
```

## 联系方式

- 项目地址: https://github.com/kzzz3/miniredis
- 邮箱: zhouke1104@qq.com

## 许可证

MIT License
