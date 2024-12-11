#pragma once
#include <chrono>
#include <thread>
#include <cassert>
#include <cstring>
#include <iostream>

#include "DataStruct/sds.h"

void test_create_empty() {
    const char* data = "";
    size_t len = strlen(data);

    Sds* obj = Sds::create(data, len, len);
    assert(obj != nullptr);
    assert(obj->length() == 0);
    assert(obj->capacity() == 0);
    assert(std::strcmp(obj->buf, data) == 0);

    Sds::destroy(obj);
}

void test_create_max_length() {
    size_t len = 255; // Maximum for uint8_t
    char data[256];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* obj = Sds::create(data, len, len);
    assert(obj != nullptr);
    assert(obj->length() == len);
    assert(obj->capacity() == len);
    assert(std::strcmp(obj->buf, data) == 0);

    Sds::destroy(obj);
}

void test_append_to_empty() {
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* obj = Sds::create("", 0, append_len);
    obj = obj->append(append_data, append_len);

    assert(std::strcmp(obj->buf, append_data) == 0);
    assert(obj->length() == append_len);
    assert(obj->capacity() >= append_len);

    Sds::destroy(obj);
}

void test_append_beyond_capacity() {
    const char* data = "Data";
    const char* append_data = "MoreData";
    size_t len = strlen(data);
    size_t append_len = strlen(append_data);

    Sds* obj = Sds::create(data, len, len);
    obj = obj->append(append_data, append_len);

    assert(std::strcmp(obj->buf, "DataMoreData") == 0);
    assert(obj->length() == len + append_len);
    assert(obj->capacity() >= len + append_len);

    Sds::destroy(obj);
}

void test_concurrent_operations() {
    const char* data = "ConcurrentTest";
    size_t len = strlen(data);

    auto create_destroy = [data, len]() {
        for (int i = 0; i < 1000; ++i) {
            Sds* obj = Sds::create(data, len, len);
            assert(obj != nullptr);
            Sds::destroy(obj);
        }
    };

    std::thread t1(create_destroy);
    std::thread t2(create_destroy);
    t1.join();
    t2.join();
}

void test_performance_create_destroy() {
    auto start = std::chrono::high_resolution_clock::now();
    const char* data = "PerformanceTest";
    size_t len = strlen(data);

    for (int i = 0; i < 1000000; ++i) {
        Sds* obj = Sds::create(data, len, len);
        Sds::destroy(obj);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Create and Destroy 1000000 objects: " << duration << " ms" << std::endl;
}

void test_performance_append() {
    auto start = std::chrono::high_resolution_clock::now();
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* obj = Sds::create("", 0, append_len);
    for (int i = 0; i < 1000000; ++i) {
        obj = obj->append(append_data, append_len);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Append 1000000 times: " << duration << " ms" << std::endl;

    Sds::destroy(obj);
}

void test_large_string() {
    size_t len = 1000000; // 1 million characters
    char* data = new char[len + 1];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* obj = Sds::create(data, len, len);
    assert(obj != nullptr);
    assert(obj->length() == len);
    assert(obj->capacity() >= len);

    Sds::destroy(obj);
    delete[] data;
}

inline void sds_tests() {
    using namespace std;

    try {
        cout << "Running detailed tests..." << endl;

        test_create_empty();
        cout << "test_create_empty passed" << endl;

        test_create_max_length();
        cout << "test_create_max_length passed" << endl;

        test_append_to_empty();
        cout << "test_append_to_empty passed" << endl;

        test_append_beyond_capacity();
        cout << "test_append_beyond_capacity passed" << endl;

        test_concurrent_operations();
        cout << "test_concurrent_operations passed" << endl;

        test_performance_create_destroy();
        test_performance_append();
        test_large_string();

        cout << "All detailed tests passed!" << endl;
    }
    catch (const std::exception& e) {
        cerr << "Test failed: " << e.what() << endl;
    }
}
