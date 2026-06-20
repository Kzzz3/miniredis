#pragma once

#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

#include "DataStruct/ziplist.h"

// ============================================================================
// Test Fixture
// ============================================================================

class ZipListTest : public ::testing::Test {
   protected:
    void SetUp() override { zl = ZipList::create(); }

    void TearDown() override {
        if (zl) {
            ZipList::destroy(zl);
            zl = nullptr;
        }
    }

    void validateZiplist() {
        uint8_t* p = zl->buf;
        for (uint16_t i = 0; i < zl->items_num; ++i) {
            ZlEntry entry;
            entryDecode(p, entry);
            if (i > 0) {
                ZlEntry prev_entry;
                entryDecode(p - entry.prevrawlen, prev_entry);
                EXPECT_EQ(prev_entry.len + prev_entry.lensize + prev_entry.prevrawlensize,
                          entry.prevrawlen)
                    << "Validation failed at index " << i;
            }
            p += entry.prevrawlensize + entry.lensize + entry.len;
        }
    }

    ZipList* zl = nullptr;
};

// ============================================================================
// Basic Operations
// ============================================================================

TEST_F(ZipListTest, CreateEmpty) {
    ASSERT_NE(zl, nullptr);
    EXPECT_EQ(zl->items_num, 0u);
}

TEST_F(ZipListTest, PushBackSingle) {
    const char* data = "hello";
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(data)), strlen(data));

    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, PushBackMultiple) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("one")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("two")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("three")), 5);

    EXPECT_EQ(zl->items_num, 3u);
    validateZiplist();
}

TEST_F(ZipListTest, PushFrontSingle) {
    const char* data = "hello";
    zl = zl->push_front(reinterpret_cast<uint8_t*>(const_cast<char*>(data)), strlen(data));

    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, PopBack) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("one")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("two")), 3);

    EXPECT_EQ(zl->items_num, 2u);

    zl = zl->pop_back();
    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, PopFront) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("one")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("two")), 3);

    EXPECT_EQ(zl->items_num, 2u);

    zl = zl->pop_front();
    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, PopAll) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("one")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("two")), 3);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("three")), 5);

    zl = zl->pop_back();
    zl = zl->pop_back();
    zl = zl->pop_back();

    EXPECT_EQ(zl->items_num, 0u);
    validateZiplist();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ZipListTest, EmptyString) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("")), 0);

    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, LargeString) {
    std::string large_str(16384, 'x');  // 16KB string
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(large_str.data())),
                        large_str.size());

    EXPECT_EQ(zl->items_num, 1u);
    validateZiplist();
}

TEST_F(ZipListTest, MixedSizes) {
    // Small
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("a")), 1);
    // Medium
    std::string medium(256, 'b');
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(medium.data())), medium.size());
    // Large
    std::string large(65536, 'c');
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(large.data())), large.size());

    EXPECT_EQ(zl->items_num, 3u);
    validateZiplist();
}

TEST_F(ZipListTest, AlternatingPushPop) {
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("one")), 3);
    EXPECT_EQ(zl->items_num, 1u);

    zl = zl->pop_back();
    EXPECT_EQ(zl->items_num, 0u);

    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("two")), 3);
    EXPECT_EQ(zl->items_num, 1u);

    zl = zl->pop_front();
    EXPECT_EQ(zl->items_num, 0u);

    validateZiplist();
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(ZipListTest, ManyItems) {
    const int NUM_ITEMS = 1000;

    for (int i = 0; i < NUM_ITEMS; ++i) {
        std::string data = "item" + std::to_string(i);
        zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.size());
    }

    EXPECT_EQ(zl->items_num, NUM_ITEMS);
    validateZiplist();
}

TEST_F(ZipListTest, PushPopCycle) {
    const int CYCLES = 100;

    for (int i = 0; i < CYCLES; ++i) {
        std::string data = "cycle" + std::to_string(i);
        zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.size());
    }

    for (int i = 0; i < CYCLES; ++i) {
        zl = zl->pop_back();
    }

    EXPECT_EQ(zl->items_num, 0u);
    validateZiplist();
}

// ============================================================================
// Performance Tests (disabled by default)
// ============================================================================

TEST_F(ZipListTest, DISABLED_PerformancePushBack) {
    const int TEST_SIZE = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_SIZE; ++i) {
        std::string data = "test" + std::to_string(i);
        zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.size());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Ziplist insert " << TEST_SIZE << " items took: " << duration << "ms\n";
}

TEST_F(ZipListTest, DISABLED_PerformancePushPop) {
    const int TEST_SIZE = 100000;

    // Fill
    for (int i = 0; i < TEST_SIZE; ++i) {
        std::string data = "test" + std::to_string(i);
        zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.size());
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Pop all
    for (int i = 0; i < TEST_SIZE; ++i) {
        zl = zl->pop_back();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Ziplist pop " << TEST_SIZE << " items took: " << duration << "ms\n";
}
