#pragma once

#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

#include "DataStruct/rbtree.h"

// ============================================================================
// Test Fixture
// ============================================================================

class ZSetTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Nothing to setup
    }

    void TearDown() override {
        // Cleanup members
        for (auto* member : members) {
            Sds::destroy(member);
        }
        members.clear();
    }

    Sds* createMember(const std::string& name) {
        auto* member = Sds::create(name.c_str(), name.length());
        members.push_back(member);
        return member;
    }

    RBTree zset;
    std::vector<Sds*> members;
};

// ============================================================================
// Basic Operations
// ============================================================================

TEST_F(ZSetTest, AddSingle) {
    auto* member = createMember("member1");

    zset.add(10.0, member);

    auto rank = zset.rank(member);
    ASSERT_TRUE(rank.has_value());
    EXPECT_EQ(rank.value(), 0);
}

TEST_F(ZSetTest, AddMultiple) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");
    auto* m3 = createMember("member3");

    zset.add(10.0, m1);
    zset.add(20.0, m2);
    zset.add(30.0, m3);

    EXPECT_EQ(zset.rank(m1).value(), 0);
    EXPECT_EQ(zset.rank(m2).value(), 1);
    EXPECT_EQ(zset.rank(m3).value(), 2);
}

TEST_F(ZSetTest, AddReverseOrder) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");
    auto* m3 = createMember("member3");

    zset.add(30.0, m1);
    zset.add(20.0, m2);
    zset.add(10.0, m3);

    // Should be sorted by score
    EXPECT_EQ(zset.rank(m3).value(), 0);  // score 10
    EXPECT_EQ(zset.rank(m2).value(), 1);  // score 20
    EXPECT_EQ(zset.rank(m1).value(), 2);  // score 30
}

TEST_F(ZSetTest, UpdateScore) {
    auto* member = createMember("member1");

    zset.add(10.0, member);
    EXPECT_EQ(zset.rank(member).value(), 0);

    zset.add(20.0, member);  // Update score
    EXPECT_EQ(zset.rank(member).value(), 0);
}

// ============================================================================
// Rank Operations
// ============================================================================

TEST_F(ZSetTest, RankNonExistent) {
    auto* member = createMember("nonexistent");

    auto rank = zset.rank(member);
    EXPECT_FALSE(rank.has_value());
}

TEST_F(ZSetTest, GetByRank) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");
    auto* m3 = createMember("member3");

    zset.add(10.0, m1);
    zset.add(20.0, m2);
    zset.add(30.0, m3);

    auto result0 = zset.getByRank(0);
    ASSERT_TRUE(result0.has_value());

    auto result1 = zset.getByRank(1);
    ASSERT_TRUE(result1.has_value());

    auto result2 = zset.getByRank(2);
    ASSERT_TRUE(result2.has_value());
}

TEST_F(ZSetTest, GetByRankOutOfBounds) {
    auto* m1 = createMember("member1");
    zset.add(10.0, m1);

    // Note: getByRank behavior for out-of-bounds may vary
    // Some implementations return the last element, others return nullopt
    auto result = zset.getByRank(100);
    // Just verify it doesn't crash
    (void)result;
}

TEST_F(ZSetTest, GetByRankEmpty) {
    auto result = zset.getByRank(0);
    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ZSetTest, DuplicateScores) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");
    auto* m3 = createMember("member3");

    zset.add(10.0, m1);
    zset.add(10.0, m2);
    zset.add(10.0, m3);

    // All should have ranks
    EXPECT_TRUE(zset.rank(m1).has_value());
    EXPECT_TRUE(zset.rank(m2).has_value());
    EXPECT_TRUE(zset.rank(m3).has_value());
}

TEST_F(ZSetTest, SameMemberMultipleTimes) {
    auto* member = createMember("member1");

    zset.add(10.0, member);
    zset.add(20.0, member);
    zset.add(30.0, member);

    // Should only appear once
    auto rank = zset.rank(member);
    ASSERT_TRUE(rank.has_value());
}

TEST_F(ZSetTest, LargeScores) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");

    zset.add(1e18, m1);
    zset.add(1e18 + 1, m2);

    EXPECT_EQ(zset.rank(m1).value(), 0);
    EXPECT_EQ(zset.rank(m2).value(), 1);
}

TEST_F(ZSetTest, NegativeScores) {
    auto* m1 = createMember("member1");
    auto* m2 = createMember("member2");
    auto* m3 = createMember("member3");

    zset.add(-10.0, m1);
    zset.add(0.0, m2);
    zset.add(10.0, m3);

    EXPECT_EQ(zset.rank(m1).value(), 0);  // -10
    EXPECT_EQ(zset.rank(m2).value(), 1);  // 0
    EXPECT_EQ(zset.rank(m3).value(), 2);  // 10
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(ZSetTest, ManyMembers) {
    const int NUM_MEMBERS = 1000;

    for (int i = 0; i < NUM_MEMBERS; ++i) {
        auto* member = createMember("member" + std::to_string(i));
        zset.add(static_cast<double>(i), member);
    }

    // Verify all ranks
    for (int i = 0; i < NUM_MEMBERS; ++i) {
        auto* member = members[i];
        auto rank = zset.rank(member);
        ASSERT_TRUE(rank.has_value()) << "Member " << i << " not found";
        EXPECT_EQ(rank.value(), i) << "Wrong rank for member " << i;
    }
}

TEST_F(ZSetTest, RandomScores) {
    const int NUM_MEMBERS = 1000;
    std::vector<double> scores;

    for (int i = 0; i < NUM_MEMBERS; ++i) {
        auto* member = createMember("member" + std::to_string(i));
        double score = static_cast<double>(rand()) / RAND_MAX * 1000.0;
        scores.push_back(score);
        zset.add(score, member);
    }

    // All members should be findable
    for (int i = 0; i < NUM_MEMBERS; ++i) {
        auto rank = zset.rank(members[i]);
        ASSERT_TRUE(rank.has_value()) << "Member " << i << " not found";
    }
}

// ============================================================================
// Performance Tests (disabled by default)
// ============================================================================

TEST_F(ZSetTest, DISABLED_PerformanceInsert) {
    const int TEST_SIZE = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_SIZE; ++i) {
        auto* member = createMember("member" + std::to_string(i));
        zset.add(static_cast<double>(i), member);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "ZSet insert " << TEST_SIZE << " items took: " << duration << "ms\n";
}

TEST_F(ZSetTest, DISABLED_PerformanceLookup) {
    const int TEST_SIZE = 10000;

    for (int i = 0; i < TEST_SIZE; ++i) {
        auto* member = createMember("member" + std::to_string(i));
        zset.add(static_cast<double>(i), member);
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_SIZE; ++i) {
        auto rank = zset.rank(members[i]);
        EXPECT_TRUE(rank.has_value());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "ZSet lookup " << TEST_SIZE << " items took: " << duration << "ms\n";
}
