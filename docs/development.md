# 开发指南

欢迎参与 MiniRedis 的开发！本指南将帮助你了解项目结构、代码规范和贡献流程。

## 开发环境

### 环境要求

- C++23 编译器 (MSVC 2022+, GCC 13+, Clang 17+)
- CMake 3.28+
- vcpkg (推荐)
- Git

### IDE 推荐

- **Visual Studio 2022** (Windows)
- **VSCode + C/C++ 扩展** (跨平台)
- **CLion** (跨平台)

## 项目结构

```
miniredis/
├── CMakeLists.txt          # 根 CMake 配置
├── CMakePresets.json        # CMake Presets 配置
├── vcpkg.json              # vcpkg 依赖配置
├── README.md               # 项目文档
├── docs/                   # 详细文档
├── miniredis/              # 源代码
│   ├── server.h/cpp        # 服务器核心
│   ├── Command/            # 命令实现
│   ├── DataStruct/         # 数据结构实现
│   ├── DataType/           # 数据类型实现
│   ├── Database/           # 数据库实现
│   ├── Networking/         # 网络层
│   └── Utility/            # 工具类
├── tests/                  # 测试代码
└── benchmark/              # 性能测试
```

## 代码规范

### 命名规范

#### 类名
- 使用 PascalCase
- 示例: `RedisObj`, `HashTable`, `Connection`

#### 函数名
- 使用 camelCase
- 示例: `create()`, `append()`, `destroy()`

#### 变量名
- 使用 snake_case
- 示例: `kvstores`, `expired_kvstores`, `deadobj`

#### 常量名
- 使用 UPPER_SNAKE_CASE
- 示例: `BUFFER_MAX_SIZE`, `DATABASE_NUM`

#### 枚举值
- 使用 UPPER_SNAKE_CASE
- 示例: `REDIS_STRING`, `REDIS_ENCODING_INT`

### 代码风格

#### 缩进
- 使用 4 个空格缩进
- 不使用 Tab

#### 大括号
- 函数、类、结构体的大括号另起一行
- if、for、while 的大括号在同一行

```cpp
// 正确
void function()
{
    if (condition) {
        // ...
    }
}

// 错误
void function() {
    if (condition) {
        // ...
    }
}
```

#### 命名空间
- 不在头文件中使用 `using namespace`
- 使用完整的命名空间路径

```cpp
// 正确
std::vector<int> vec;
asio::io_context io;

// 错误
using namespace std;
using namespace asio;
```

### 注释规范

#### 文件头注释
```cpp
// 文件名: xxx.h
// 描述: xxx
// 作者: xxx
// 日期: xxx
```

#### 函数注释
```cpp
/**
 * @brief 函数描述
 * @param param1 参数1描述
 * @param param2 参数2描述
 * @return 返回值描述
 */
```

#### 行内注释
```cpp
// 这是行内注释
int x = 10;  // 这是行尾注释
```

## 添加新命令

### 步骤

1. **创建命令函数**

在相应的命令文件中添加函数：

```cpp
// stringcmd.cpp
bool CmdNewCommand(shared_ptr<Connection> conn, Command& cmd)
{
    // 参数验证
    if (cmd.size() != 2)
        return false;
    
    // 实现逻辑
    // ...
    
    // 返回结果
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}
```

2. **声明命令函数**

在 `command.h` 中添加声明：

```cpp
// string command
bool CmdNewCommand(shared_ptr<Connection> conn, Command& cmd);
```

3. **注册命令**

在 `commands_map` 中添加命令：

```cpp
inline const unordered_map<string, function<bool(shared_ptr<Connection>, Command&)>> commands_map = {
    // ...
    {"newcommand", CmdNewCommand},
    // ...
};
```

4. **添加测试**

在测试文件中添加测试用例：

```cpp
TEST_F(StringCommandTest, NewCommand) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }
    
    sendCommand({"NEWCOMMAND", "arg1"});
    std::string response = readResponse();
    
    EXPECT_NE(response.find("OK"), std::string::npos);
}
```

## 添加新数据结构

### 步骤

1. **创建头文件**

在 `DataStruct/` 目录下创建头文件：

```cpp
// DataStruct/newstruct.h
#pragma once

class NewStruct {
public:
    // 接口定义
    void method1();
    int method2();
    
private:
    // 内部数据
};
```

2. **实现数据结构**

在 `DataStruct/` 目录下创建实现文件：

```cpp
// DataStruct/newstruct.cpp
#include "newstruct.h"

void NewStruct::method1()
{
    // 实现
}

int NewStruct::method2()
{
    // 实现
    return 0;
}
```

3. **创建数据类型**

在 `DataType/` 目录下创建数据类型文件：

```cpp
// DataType/newtypeobj.hpp
#pragma once

#include "redisobj.h"
#include "DataStruct/newstruct.h"

inline RedisObj* NewTypeObjectCreate()
{
    RedisObj* obj = Allocator::create<RedisObj>();
    obj->type = ObjType::REDIS_NEWTYPE;
    obj->encoding = ObjEncoding::REDIS_ENCODING_NEWTYPE;
    obj->data.ptr = Allocator::create<NewStruct>();
    obj->lru = GetSecTimestamp();
    obj->refcount = 1;
    return obj;
}
```

4. **更新枚举**

在 `redisobj.h` 中更新枚举：

```cpp
enum class ObjType : uint8_t {
    REDIS_STRING,
    REDIS_LIST,
    REDIS_HASH,
    REDIS_SET,
    REDIS_ZSET,
    REDIS_NEWTYPE  // 新增
};

enum class ObjEncoding : uint8_t {
    // ...
    REDIS_ENCODING_NEWTYPE,  // 新增
};
```

## 运行测试

### 运行所有测试

```bash
# 使用 CTest
ctest --preset default
```

### 运行特定测试

```bash
# 运行数据结构测试
./build/tests/DataStructTest/Debug/DataStructTest.exe

# 运行数据类型测试 (需要服务器运行)
./build/tests/DataTypeTest/Debug/DataTypeTest.exe
```

### 运行单个测试

```bash
# 运行特定测试用例
./build/tests/DataStructTest/Debug/DataStructTest.exe --gtest_filter="SdsTest.CreateEmpty"
```

## 调试技巧

### 使用调试器

#### Visual Studio
1. 设置断点
2. 按 F5 启动调试

#### VSCode
1. 安装 C/C++ 扩展
2. 配置 launch.json
3. 按 F5 启动调试

#### GDB
```bash
# 编译 Debug 版本
cmake --preset debug
cmake --build --preset debug

# 使用 GDB 调试
gdb ./build/miniredis/Debug/miniredis.exe
```

### 日志输出

使用 `std::cout` 输出调试信息：

```cpp
std::cout << "Debug: " << value << std::endl;
```

### 内存检查

使用 Valgrind 检查内存泄漏：

```bash
valgrind --leak-check=full ./build/miniredis/Debug/miniredis.exe
```

## 贡献流程

### 1. Fork 项目

在 GitHub 上 Fork 项目到你的账户。

### 2. 克隆仓库

```bash
git clone https://github.com/yourusername/miniredis.git
cd miniredis
```

### 3. 创建特性分支

```bash
git checkout -b feature/AmazingFeature
```

### 4. 提交更改

```bash
git add .
git commit -m "feat: 添加 AmazingFeature"
```

### 5. 推送到分支

```bash
git push origin feature/AmazingFeature
```

### 6. 创建 Pull Request

在 GitHub 上创建 Pull Request，描述你的更改。

## 提交规范

### 提交消息格式

```
<type>(<scope>): <subject>

<body>

<footer>
```

### 类型

- `feat`: 新功能
- `fix`: 修复 bug
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 重构
- `test`: 测试相关
- `chore`: 构建/工具相关

### 示例

```
feat(command): 添加 MGET 命令

- 实现 MGET 命令，支持批量获取多个 key
- 添加相关测试用例

Closes #123
```

## 代码审查

### 审查要点

- 代码风格是否符合规范
- 是否有潜在的内存泄漏
- 是否有线程安全问题
- 测试是否充分
- 文档是否完整

### 审查流程

1. 提交 Pull Request
2. 等待审查
3. 根据反馈修改
4. 合并到主分支

## 发布流程

### 版本号

使用语义化版本号：`主版本号.次版本号.修订号`

- 主版本号: 不兼容的 API 修改
-次版本号: 向下兼容的功能性新增
- 修订号: 向下兼容的问题修正

### 发布步骤

1. 更新版本号
2. 更新 CHANGELOG
3. 创建 Git 标签
4. 发布到 GitHub

## 常见问题

### Q: 如何添加新的数据类型？

A: 参考 "添加新数据结构" 章节。

### Q: 如何添加新的命令？

A: 参考 "添加新命令" 章节。

### Q: 如何运行测试？

A: 参考 "运行测试" 章节。

### Q: 如何调试？

A: 参考 "调试技巧" 章节。

## 联系方式

- 项目地址: https://github.com/kzzz3/miniredis
- 邮箱: zhouke1104@qq.com
