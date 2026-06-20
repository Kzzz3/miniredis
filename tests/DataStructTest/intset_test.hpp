#pragma once

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <vector>

#include "DataStruct/intset.h"

// ============================================================================
// Basic Operations
// ============================================================================

TEST(IntSetTest, CreateEmpty) {
    IntSet* is = IntSet::create();
    ASSERT_NE(is, nullptr);
    EXPECT_EQ(is->length, 0u);

    IntSet::destroy(is);
}

TEST(IntSetTest, InsertSingle) {
    IntSet* is = IntSet::create();

    is = is->insert(42);
    EXPECT_TRUE(is->contains(42));
    EXPECT_FALSE(is->contains(0));
    EXPECT_FALSE(is->contains(100));

    IntSet::destroy(is);
}

TEST(IntSetTest, InsertMultiple) {
    IntSet* is = IntSet::create();

    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);
    is = is->insert(15);
    is = is->insert(25);

    EXPECT_TRUE(is->contains(5));
    EXPECT_TRUE(is->contains(10));
    EXPECT_TRUE(is->contains(15));
    EXPECT_TRUE(is->contains(20));
    EXPECT_TRUE(is->contains(25));
    EXPECT_FALSE(is->contains(0));
    EXPECT_FALSE(is->contains(30));

    IntSet::destroy(is);
}

TEST(IntSetTest, InsertDuplicate) {
    IntSet* is = IntSet::create();

    is = is->insert(42);
    is = is->insert(42);  // Duplicate insert

    EXPECT_TRUE(is->contains(42));
    // Note: behavior may vary - some implementations allow duplicates, some don't

    IntSet::destroy(is);
}

TEST(IntSetTest, InsertNegative) {
    IntSet* is = IntSet::create();

    is = is->insert(-10);
    is = is->insert(-5);
    is = is->insert(0);
    is = is->insert(5);
    is = is->insert(10);

    EXPECT_TRUE(is->contains(-10));
    EXPECT_TRUE(is->contains(-5));
    EXPECT_TRUE(is->contains(0));
    EXPECT_TRUE(is->contains(5));
    EXPECT_TRUE(is->contains(10));

    IntSet::destroy(is);
}

// ============================================================================
// Encoding Upgrades
// ============================================================================

TEST(IntSetTest, UpgradeInt16ToInt32) {
    IntSet* is = IntSet::create();

    is = is->insert(INT16_MAX);
    EXPECT_EQ(is->encoding, INTSET_ENC_INT16);

    is = is->insert(INT16_MAX + 1);
    EXPECT_EQ(is->encoding, INTSET_ENC_INT32);

    EXPECT_TRUE(is->contains(INT16_MAX));
    EXPECT_TRUE(is->contains(INT16_MAX + 1));

    IntSet::destroy(is);
}

TEST(IntSetTest, UpgradeInt32ToInt64) {
    IntSet* is = IntSet::create();

    is = is->insert(INT32_MAX);
    EXPECT_EQ(is->encoding, INTSET_ENC_INT32);

    is = is->insert((int64_t)INT32_MAX + 1);
    EXPECT_EQ(is->encoding, INTSET_ENC_INT64);

    EXPECT_TRUE(is->contains(INT32_MAX));
    EXPECT_TRUE(is->contains((int64_t)INT32_MAX + 1));

    IntSet::destroy(is);
}

TEST(IntSetTest, DirectInt64) {
    IntSet* is = IntSet::create();

    is = is->insert(INT64_MAX);
    EXPECT_EQ(is->encoding, INTSET_ENC_INT64);
    EXPECT_TRUE(is->contains(INT64_MAX));

    is = is->insert(INT64_MIN);
    EXPECT_TRUE(is->contains(INT64_MIN));

    IntSet::destroy(is);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(IntSetTest, BoundaryValues) {
    IntSet* is = IntSet::create();

    // Test INT8 boundaries
    is = is->insert(INT8_MIN);
    is = is->insert(INT8_MAX);
    EXPECT_TRUE(is->contains(INT8_MIN));
    EXPECT_TRUE(is->contains(INT8_MAX));

    // Test INT16 boundaries
    is = is->insert(INT16_MIN);
    is = is->insert(INT16_MAX);
    EXPECT_TRUE(is->contains(INT16_MIN));
    EXPECT_TRUE(is->contains(INT16_MAX));

    IntSet::destroy(is);
}

TEST(IntSetTest, LargeNumberOfElements) {
    IntSet* is = IntSet::create();
    const int NUM_ELEMENTS = 10000;

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        is = is->insert(i);
    }

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        EXPECT_TRUE(is->contains(i)) << "Failed to find " << i;
    }

    for (int i = NUM_ELEMENTS; i < NUM_ELEMENTS + 100; ++i) {
        EXPECT_FALSE(is->contains(i)) << "Should not find " << i;
    }

    IntSet::destroy(is);
}

// ============================================================================
// Performance Tests (disabled by default)
// ============================================================================

TEST(IntSetTest, DISABLED_PerformanceInsert) {
    IntSet* is = IntSet::create();
    const int TEST_SIZE = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_SIZE; ++i) {
        is = is->insert(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "IntSet insert " << TEST_SIZE << " items took: " << duration << "ms\n";

    IntSet::destroy(is);
}

TEST(IntSetTest, DISABLED_PerformanceLookup) {
    IntSet* is = IntSet::create();
    const int TEST_SIZE = 100000;

    for (int i = 0; i < TEST_SIZE; ++i) {
        is = is->insert(i);
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_SIZE; ++i) {
        EXPECT_TRUE(is->contains(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "IntSet lookup " << TEST_SIZE << " items took: " << duration << "ms\n";

    IntSet::destroy(is);
}
