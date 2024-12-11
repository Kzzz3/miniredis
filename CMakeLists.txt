cmake_minimum_required(VERSION 3.2)
project(miniredis VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 vcpkg 工具链文件路径
set(CMAKE_PREFIX_PATH "/home/kzzz3/vcpkg/installed/x64-linux")
set(CMAKE_TOOLCHAIN_FILE "/home/kzzz3/vcpkg/scripts/buildsystems/vcpkg.cmake")

# 包含子目录
add_subdirectory(tests)
add_subdirectory(benchmark)
add_subdirectory(miniredis)