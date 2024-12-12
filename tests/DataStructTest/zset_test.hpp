#pragma once
#include "DataStruct/rbtree.h"
#include <cassert>
#include <chrono>
#include <iostream>

inline void testZSetFunctionality()
{
    RBTree zset;

    auto member1 = Sds::create("member1", 7);
    auto member2 = Sds::create("member2", 7);
    auto member3 = Sds::create("member3", 7);

    // Test adding members
    zset.add(10, member1);
    zset.add(20, member2);
    assert(zset.rank(member1).value() == 0);
    assert(zset.rank(member2).value() == 1);

    // Test updating scores
    zset.add(30, member1);
    assert(zset.rank(member1).value() == 1);

    // Test removing members
    zset.remove(member1);
    assert(zset.contains(member1) == false);
    assert(zset.rank(member1) == std::nullopt);

    // Test retrieving members by rank
    auto result = zset.getByRank(0);
    assert(result.has_value());
    assert(result->second == member2);

    // Test range queries
    zset.add(10, member1);
    zset.add(25, member3);
    auto range = zset.rangeByScore(15, 30);
    assert(range.size() == 2); // member2, member3

    if (zset.rbt.size() != 0)
    {
        auto first = zset.getByRank(0);
        auto last = zset.getByRank(-1);

        auto entrys = zset.rangeByScore(first->first, last->first);
        for (auto& [score, member] : entrys)
        {
            Sds::destroy(member);
        }
    }
}

inline void testZSetEdgeCases()
{
    RBTree zset;

    auto member1 = Sds::create("member1", 7);

    // Test operations on empty set
    assert(zset.rank(member1) == std::nullopt);
    assert(zset.getByRank(0) == std::nullopt);

    // Test removing non-existent members
    zset.remove(member1);
    assert(zset.contains(member1) == false);

    // Test adding duplicate members
    zset.add(10, member1);
    zset.add(10, member1); // Duplicate
    assert(zset.rank(member1).value() == 0);

    // Test boundary conditions for range queries
    auto range = zset.rangeByScore(5, 10);
    assert(range.size() == 1); // Only member1

    if (zset.rbt.size() != 0)
    {
        auto first = zset.getByRank(0);
        auto last = zset.getByRank(-1);

        auto entrys = zset.rangeByScore(first->first, last->first);
        for (auto& [score, member] : entrys)
        {
            Sds::destroy(member);
        }
    }
}

inline void testZSetPerformance()
{
    RBTree zset;

    constexpr size_t NUM_ELEMENTS = 10000;
    std::vector<Sds*> members;

    // Insert elements into the set
    for (size_t i = 0; i < NUM_ELEMENTS; ++i)
    {
        std::string temp = "member" + std::to_string(i);
        auto member = Sds::create(temp.c_str(), temp.length());
        members.push_back(member);
        zset.add(i, member);
    }

    // Test rank performance
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < NUM_ELEMENTS; ++i)
    {
        assert(zset.rank(members[i]).value() == i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Rank test completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";

    // Test range query performance
    start = std::chrono::high_resolution_clock::now();
    auto range = zset.rangeByScore(100, 200);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "RangeByScore test completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms, found " << range.size() << " elements\n";

    // Test deletion performance
    start = std::chrono::high_resolution_clock::now();
    for (auto member : members)
    {
        zset.remove(member);
        Sds::destroy(member);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Delete test completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";

    if (zset.rbt.size() != 0)
    {
        auto first = zset.getByRank(0);
        auto last = zset.getByRank(-1);

        auto entrys = zset.rangeByScore(first->first, last->first);
        for (auto& [score, member] : entrys)
        {
            Sds::destroy(member);
        }
    }
}

inline void zset_test()
{
    std::cout << "Testing ZSet functionality...\n";
    testZSetFunctionality();
    std::cout << "Functionality test passed!\n";

    std::cout << "Testing ZSet edge cases...\n";
    testZSetEdgeCases();
    std::cout << "Edge cases test passed!\n";

    std::cout << "Testing ZSet performance...\n";
    testZSetPerformance();
    std::cout << "Performance test completed!\n";
}
