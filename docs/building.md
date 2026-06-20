# 构建指南

## 环境要求

### 编译器

- **Windows**: MSVC 2022+ (Visual Studio 2022 或更高版本)
- **Linux**: GCC 13+ 或 Clang 17+
- **macOS**: Xcode 15+ (Apple Clang 15+)

### 构建工具

- **CMake**: 3.28 或更高版本
- **vcpkg**: 推荐使用 vcpkg 管理依赖

## 安装依赖

### Windows (使用 vcpkg)

```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 设置环境变量
set VCPKG_ROOT=C:\path\to\vcpkg

# 安装依赖
vcpkg install
```

### Linux (使用 vcpkg)

```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# 设置环境变量
export VCPKG_ROOT=/path/to/vcpkg

# 安装依赖
./vcpkg install
```

### macOS (使用 vcpkg)

```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# 设置环境变量
export VCPKG_ROOT=/path/to/vcpkg

# 安装依赖
./vcpkg install
```

## 配置项目

### 使用 CMake Presets (推荐)

项目支持 CMake Presets，简化构建流程：

```bash
# 配置 Debug 版本
cmake --preset debug

# 配置 Release 版本
cmake --preset release
```

### 手动配置

```bash
# 创建构建目录
mkdir build
cd build

# 配置 (Debug)
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
         -DCMAKE_BUILD_TYPE=Debug

# 配置 (Release)
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
         -DCMAKE_BUILD_TYPE=Release
```

### 配置 CMakeUserPresets.json

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

## 构建项目

### 使用 CMake Presets

```bash
# 构建 Debug 版本
cmake --build --preset debug

# 构建 Release 版本
cmake --build --preset release
```

### 手动构建

```bash
# 进入构建目录
cd build

# 构建
cmake --build . --config Debug
# 或
cmake --build . --config Release
```

## 运行程序

### 运行服务器

```bash
# Debug 版本
./build/miniredis/Debug/miniredis.exe

# Release 版本
./build/miniredis/Release/miniredis.exe
```

### 使用 redis-cli 连接

```bash
redis-cli -p 10087
```

## 运行测试

### 使用 CTest

```bash
# 运行所有测试
ctest --preset default

# 运行特定测试
ctest --preset default -R DataStructTest
ctest --preset default -R DataTypeTest
```

### 直接运行测试程序

```bash
# 运行数据结构测试
./build/tests/DataStructTest/Debug/DataStructTest.exe

# 运行数据类型测试 (需要服务器运行)
./build/tests/DataTypeTest/Debug/DataTypeTest.exe
```

## 常见问题

### 1. 找不到 vcpkg

确保设置了 `VCPKG_ROOT` 环境变量，或者在 `CMakeUserPresets.json` 中配置。

### 2. 编译错误

确保使用支持 C++23 的编译器：
- Windows: MSVC 2022 或更高版本
- Linux: GCC 13+ 或 Clang 17+
- macOS: Xcode 15+

### 3. 链接错误

确保所有依赖都已正确安装：

```bash
vcpkg install
```

### 4. 测试失败

确保服务器正在运行：

```bash
# 启动服务器
./build/miniredis/Debug/miniredis.exe

# 在另一个终端运行测试
./build/tests/DataTypeTest/Debug/DataTypeTest.exe
```

## IDE 配置

### Visual Studio

1. 打开 Visual Studio
2. 选择 "打开本地文件夹"
3. 选择项目根目录
4. Visual Studio 会自动检测 CMakeLists.txt

### VSCode

1. 安装 C/C++ 扩展
2. 安装 CMake Tools 扩展
3. 打开项目文件夹
4. 选择 CMake Preset

### CLion

1. 打开 CLion
2. 选择 "Open"
3. 选择项目根目录
4. CLion 会自动检测 CMakeLists.txt

## 构建选项

### CMake 选项

| 选项 | 描述 | 默认值 |
|------|------|--------|
| `CMAKE_BUILD_TYPE` | 构建类型 | Debug |
| `CMAKE_CXX_STANDARD` | C++ 标准 | 23 |
| `VCPKG_TARGET_TRIPLET` | vcpkg triplet | 系统默认 |

### vcpkg 选项

在 `vcpkg.json` 中可以配置依赖：

```json
{
  "dependencies": [
    "asio",
    "yalantinglibs",
    "zlib",
    "jemalloc",
    "gtest"
  ]
}
```
