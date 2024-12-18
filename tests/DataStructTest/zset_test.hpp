#pragma once
#include "DataStruct/rbtree.h"
#include <cassert>
#include <chrono>

// Test basic zset operations
inline void test_zset_basic_ops()
{
    RBTree zset;

    // Create test members
    auto member1 = Sds::create("member1", 7);
    auto member2 = Sds::create("member2", 7);

    // Test add and rank operations
    zset.add(10, member1);
    zset.add(20, member2);
    assert(zset.rank(member1).value() == 0);
    assert(zset.rank(member2).value() == 1);

    // Cleanup
    Sds::destroy(member1);
    Sds::destroy(member2);
}

// Test edge cases
inline void test_zset_edge_cases()
{
    RBTree zset;

    // Test empty set operations
    auto member = Sds::create("test", 4);
    assert(zset.rank(member) == std::nullopt);
    assert(zset.getByRank(0) == std::nullopt);

    // Test duplicate scores
    zset.add(10, member);
    zset.add(10, member);
    assert(zset.rank(member).value() == 0);

    Sds::destroy(member);
}

// Test performance
inline void test_zset_performance()
{
    RBTree zset;
    const int TEST_SIZE = 10000;
    std::vector<Sds*> members;

    auto start = std::chrono::high_resolution_clock::now();

    // Insert test
    for (int i = 0; i < TEST_SIZE; ++i)
    {
        std::string name = "member" + std::to_string(i);
        auto member = Sds::create(name.c_str(), name.length());
        members.push_back(member);
        zset.add(i, member);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "ZSet insert " << TEST_SIZE << " items took: " << duration << "ms\n";

    // Cleanup
    for (auto member : members)
    {
        Sds::destroy(member);
    }
}

// Main test runner for zset
inline void run_zset_tests()
{
    try
    {
        std::cout << "Running basic operations tests...\n";
        test_zset_basic_ops();
        std::cout << "✓ Basic operations tests passed\n\n";

        std::cout << "Running edge cases tests...\n";
        test_zset_edge_cases();
        std::cout << "✓ Edge cases tests passed\n\n";

        std::cout << "Running performance tests...\n";
        test_zset_performance();
        std::cout << "✓ Performance tests passed\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        throw;
    }
}