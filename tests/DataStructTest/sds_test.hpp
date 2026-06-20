#pragma once

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>

#include "DataStruct/sds.h"

// ============================================================================
// Basic Creation Tests
// ============================================================================

TEST(SdsTest, CreateEmpty) {
    const char* data = "";
    size_t len = strlen(data);

    Sds* sds = Sds::create(data, len, len);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), 0u);
    EXPECT_EQ(sds->capacity(), 0u);
    EXPECT_STREQ(sds->buf, data);

    Sds::destroy(sds);
}

TEST(SdsTest, CreateWithContent) {
    const char* data = "Hello, World!";
    size_t len = strlen(data);

    Sds* sds = Sds::create(data, len, len);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), len);
    EXPECT_EQ(sds->capacity(), len);
    EXPECT_STREQ(sds->buf, data);

    Sds::destroy(sds);
}

TEST(SdsTest, CreateMaxLength) {
    size_t len = 255;  // Maximum for uint8_t
    char data[256];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* sds = Sds::create(data, len, len);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), len);
    EXPECT_EQ(sds->capacity(), len);
    EXPECT_STREQ(sds->buf, data);

    Sds::destroy(sds);
}

TEST(SdsTest, CreateWithExtraCapacity) {
    const char* data = "Short";
    size_t len = strlen(data);
    size_t capacity = 100;

    Sds* sds = Sds::create(data, len, capacity);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), len);
    EXPECT_GE(sds->capacity(), capacity);
    EXPECT_STREQ(sds->buf, data);

    Sds::destroy(sds);
}

// ============================================================================
// Append Tests
// ============================================================================

TEST(SdsTest, AppendToEmpty) {
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create("", 0, append_len);
    sds = sds->append(append_data, append_len);

    EXPECT_STREQ(sds->buf, append_data);
    EXPECT_EQ(sds->length(), append_len);
    EXPECT_GE(sds->capacity(), append_len);

    Sds::destroy(sds);
}

TEST(SdsTest, AppendWithinCapacity) {
    const char* data = "Hello";
    const char* append = " World";
    size_t len = strlen(data);
    size_t append_len = strlen(append);
    size_t capacity = 100;  // Large enough to avoid reallocation

    Sds* sds = Sds::create(data, len, capacity);
    sds = sds->append(append, append_len);

    EXPECT_STREQ(sds->buf, "Hello World");
    EXPECT_EQ(sds->length(), len + append_len);

    Sds::destroy(sds);
}

TEST(SdsTest, AppendBeyondCapacity) {
    const char* data = "Data";
    const char* append_data = "MoreData";
    size_t len = strlen(data);
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create(data, len, len);
    sds = sds->append(append_data, append_len);

    EXPECT_STREQ(sds->buf, "DataMoreData");
    EXPECT_EQ(sds->length(), len + append_len);
    EXPECT_GE(sds->capacity(), len + append_len);

    Sds::destroy(sds);
}

TEST(SdsTest, MultipleAppends) {
    Sds* sds = Sds::create("", 0, 10);

    sds = sds->append("A", 1);
    EXPECT_STREQ(sds->buf, "A");

    sds = sds->append("B", 1);
    EXPECT_STREQ(sds->buf, "AB");

    sds = sds->append("C", 1);
    EXPECT_STREQ(sds->buf, "ABC");

    EXPECT_EQ(sds->length(), 3u);

    Sds::destroy(sds);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(SdsTest, AppendEmptyString) {
    const char* data = "Hello";
    size_t len = strlen(data);

    Sds* sds = Sds::create(data, len, len);
    sds = sds->append("", 0);

    EXPECT_STREQ(sds->buf, "Hello");
    EXPECT_EQ(sds->length(), len);

    Sds::destroy(sds);
}

TEST(SdsTest, LargeString) {
    size_t len = 1000000;  // 1 million characters
    char* data = new char[len + 1];
    std::fill(data, data + len, 'A');
    data[len] = '\0';

    Sds* sds = Sds::create(data, len, len);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), len);
    EXPECT_GE(sds->capacity(), len);

    Sds::destroy(sds);
    delete[] data;
}

TEST(SdsTest, BinaryData) {
    char data[5] = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    size_t len = 5;

    Sds* sds = Sds::create(data, len, len);
    ASSERT_NE(sds, nullptr);
    EXPECT_EQ(sds->length(), len);
    EXPECT_EQ(memcmp(sds->buf, data, len), 0);

    Sds::destroy(sds);
}

// ============================================================================
// Concurrent Operations
// ============================================================================

TEST(SdsTest, ConcurrentCreateDestroy) {
    const char* data = "ConcurrentTest";
    size_t len = strlen(data);

    auto create_destroy = [data, len]() {
        for (int i = 0; i < 1000; ++i) {
            Sds* sds = Sds::create(data, len, len);
            ASSERT_NE(sds, nullptr);
            Sds::destroy(sds);
        }
    };

    std::thread t1(create_destroy);
    std::thread t2(create_destroy);
    t1.join();
    t2.join();
}

// ============================================================================
// Performance Tests (disabled by default)
// ============================================================================

TEST(SdsTest, DISABLED_PerformanceCreateDestroy) {
    auto start = std::chrono::high_resolution_clock::now();
    const char* data = "PerformanceTest";
    size_t len = strlen(data);

    for (int i = 0; i < 1000000; ++i) {
        Sds* sds = Sds::create(data, len, len);
        Sds::destroy(sds);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Create and Destroy 1000000 objects: " << duration
              << " ms" << std::endl;
}

TEST(SdsTest, DISABLED_PerformanceAppend) {
    auto start = std::chrono::high_resolution_clock::now();
    const char* append_data = "AppendMe";
    size_t append_len = strlen(append_data);

    Sds* sds = Sds::create("", 0, append_len);
    for (int i = 0; i < 1000000; ++i) {
        sds = sds->append(append_data, append_len);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Performance Test - Append 1000000 times: " << duration << " ms"
              << std::endl;

    Sds::destroy(sds);
}
