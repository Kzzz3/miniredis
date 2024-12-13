# 设置测试源文件路径
set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 收集测试源文件
file(GLOB TEST_SOURCES

    ${TEST_SRC_DIR}/*.cpp
    ${TEST_SRC_DIR}/*.hpp
)

# 创建测试可执行文件
add_executable(DataTypeTest ${TEST_SOURCES})

# 链接 Asio 库
find_package(asio CONFIG REQUIRED)
target_link_libraries(DataTypeTest PRIVATE asio)

# 添加测试到 CTest
add_test(NAME DataTypeTest COMMAND DataTypeTest)
