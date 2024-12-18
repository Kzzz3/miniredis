# 子项目配置
set(MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 包含子目录头文件
include_directories(
    ${MAIN_SRC_DIR}
    ${MAIN_SRC_DIR}/Command
    ${MAIN_SRC_DIR}/DataStruct
    ${MAIN_SRC_DIR}/DataType
    ${MAIN_SRC_DIR}/Networking
    ${MAIN_SRC_DIR}/Utility
)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    ${MAIN_SRC_DIR}/*.cpp
    ${MAIN_SRC_DIR}/*.hpp
    ${MAIN_SRC_DIR}/*.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接 Asio 库
find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE asio)

# 链接 Yalantinglibs
find_package(yalantinglibs CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yalantinglibs::yalantinglibs)

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

# 添加编译器选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()