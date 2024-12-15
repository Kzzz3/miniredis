# 设置测试源文件路径
set(BENCHMARK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 收集测试源文件
file(GLOB BENCHMARK_SOURCES

    ${BENCHMARK_SRC_DIR}/*.cpp
    ${BENCHMARK_SRC_DIR}/*.hpp
)

# 创建测试可执行文件
add_executable(Benchmark ${BENCHMARK_SOURCES})

# 链接 Asio 库
find_package(asio CONFIG REQUIRED)
target_link_libraries(Benchmark PRIVATE asio)