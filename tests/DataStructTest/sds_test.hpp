#pragma once
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#include "DataStruct/sds.h"

// Test basic creation with empty string
void test_sds_create_empty()
{
    const char* data = "";
    size_t len = strlen(data);

    Sds* sds = Sds::create(data, len, len);
    assert(sds != nullptr);
    assert(sds->length() == 0);
    assert(sds->capacity() == 0);
    assert(std::strcmp(sds->buf, data) == 0);

    Sds::destroy(sds);
}

void test_sds_create_max_length()
{
    size_t len = 255; // Maximum for uint8_t
    char data[256];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* sds = Sds::create(data, len, len);
    assert(sds != nullptr);
    assert(sds->length() == len);
    assert(sds->capacity() == len);
    assert(std::strcmp(sds->buf, data) == 0);

    Sds::destroy(sds);
}

void test_sds_append_to_empty()
{
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create("", 0, append_len);
    sds = sds->append(append_data, append_len);

    assert(std::strcmp(sds->buf, append_data) == 0);
    assert(sds->length() == append_len);
    assert(sds->capacity() >= append_len);

    Sds::destroy(sds);
}

void test_sds_append_beyond_capacity()
{
    const char* data = "Data";
    const char* append_data = "MoreData";
    size_t len = strlen(data);
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create(data, len, len);
    sds = sds->append(append_data, append_len);

    assert(std::strcmp(sds->buf, "DataMoreData") == 0);
    assert(sds->length() == len + append_len);
    assert(sds->capacity() >= len + append_len);

    Sds::destroy(sds);
}

void test_sds_concurrent_operations()
{
    const char* data = "ConcurrentTest";
    size_t len = strlen(data);

    auto create_destroy = [data, len]()
    {
        for (int i = 0; i < 1000; ++i)
        {
            Sds* sds = Sds::create(data, len, len);
            assert(sds != nullptr);
            Sds::destroy(sds);
        }
    };

    std::thread t1(create_destroy);
    std::thread t2(create_destroy);
    t1.join();
    t2.join();
}

void test_sds_performance_create_destroy()
{
    auto start = std::chrono::high_resolution_clock::now();
    const char* data = "PerformanceTest";
    size_t len = strlen(data);

    for (int i = 0; i < 1000000; ++i)
    {
        Sds* sds = Sds::create(data, len, len);
        Sds::destroy(sds);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Create and Destroy 1000000 objects: " << duration << " ms"
              << std::endl;
}

void test_sds_performance_append()
{
    auto start = std::chrono::high_resolution_clock::now();
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create("", 0, append_len);
    std::string str(900000, 'a');
    // for (int i = 0; i < 1000000; ++i) {
    //     sds = sds->append(append_data, append_len);
    // }
    sds = sds->append(str.c_str(), str.length());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Append 1000000 times: " << duration << " ms" << std::endl;

    Sds::destroy(sds);
}

void test_sds_large_string()
{
    size_t len = 1000000; // 1 million characters
    char* data = new char[len + 1];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* sds = Sds::create(data, len, len);
    assert(sds != nullptr);
    assert(sds->length() == len);
    assert(sds->capacity() >= len);

    Sds::destroy(sds);
    delete[] data;
}

// Main test runner for SDS
inline void run_sds_tests()
{
    try
    {
        // Basic operations
        std::cout << "Running basic operations tests...\n";
        test_sds_create_empty();
        test_sds_create_max_length();
        std::cout << "✓ Basic operations tests passed\n\n";

        // String operations
        std::cout << "Running string operations tests...\n";
        test_sds_append_to_empty();
        test_sds_append_beyond_capacity();
        std::cout << "✓ String operations tests passed\n\n";

        // Concurrent operations
        std::cout << "Running concurrent operations tests...\n";
        test_sds_concurrent_operations();
        std::cout << "✓ Concurrent operations tests passed\n\n";

        // Performance tests
        std::cout << "Running performance tests...\n";
        test_sds_performance_create_destroy();
        test_sds_performance_append();
        test_sds_large_string();
        std::cout << "✓ Performance tests passed\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        throw;
    }
}