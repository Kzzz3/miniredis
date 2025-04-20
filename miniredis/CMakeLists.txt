# 子项目配置
set(MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 包含子目录头文件
include_directories(
    ${MAIN_SRC_DIR}
    ${MAIN_SRC_DIR}/DataStruct
    ${MAIN_SRC_DIR}/Networking
    ${MAIN_SRC_DIR}/Database
    ${MAIN_SRC_DIR}/DataType
    ${MAIN_SRC_DIR}/Utility
    ${MAIN_SRC_DIR}/Command
)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    ${MAIN_SRC_DIR}/*.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接 Asio 库
find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE asio)

# 链接 Yalantinglibs
find_package(yalantinglibs CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yalantinglibs::yalantinglibs)

# 链接 ZLIB 库
find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

# 链接 Jemalloc
find_package(jemalloc CONFIG QUIET)
if(jemalloc_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE jemalloc::jemalloc)
else()
    find_path(JEMALLOC_INCLUDE_DIR NAMES jemalloc/jemalloc.h HINTS "${CMAKE_PREFIX_PATH}/include")
    find_library(JEMALLOC_LIBRARY NAMES jemalloc libjemalloc HINTS "${CMAKE_PREFIX_PATH}/lib")
    if(JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIBRARY)
        message(STATUS "Found jemalloc: ${JEMALLOC_LIBRARY}")
        add_library(jemalloc INTERFACE)
        target_include_directories(jemalloc INTERFACE ${JEMALLOC_INCLUDE_DIR})
        target_link_libraries(jemalloc INTERFACE ${JEMALLOC_LIBRARY})
        target_link_libraries(${PROJECT_NAME} PRIVATE jemalloc)
    else()
        message(FATAL_ERROR "jemalloc not found. Please ensure it is installed via vcpkg.")
    endif()
endif()

# 添加编译器选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()