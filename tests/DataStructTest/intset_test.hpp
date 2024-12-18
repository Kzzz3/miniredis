#pragma once
#include "DataStruct/intset.h"
#include <cassert>
#include <chrono>

// Test basic intset operations
inline void test_intset_basic_ops()
{
    IntSet* is = IntSet::create();

    // Test insertion
    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);

    assert(is->contains(10));
    assert(is->contains(5));
    assert(is->contains(20));
    assert(!is->contains(15));

    IntSet::destroy(is);
}

// Test encoding upgrades
inline void test_intset_upgrades()
{
    IntSet* is = IntSet::create();

    // Test encoding upgrades
    is = is->insert(INT16_MAX);
    is = is->insert(INT32_MAX);
    assert(is->encoding == INTSET_ENC_INT32);

    is = is->insert(INT64_MAX);
    assert(is->encoding == INTSET_ENC_INT64);

    IntSet::destroy(is);
}

// Test performance
inline void test_intset_performance()
{
    IntSet* is = IntSet::create();
    const int TEST_SIZE = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    // Insert test
    for (int i = 0; i < TEST_SIZE; ++i)
    {
        is = is->insert(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "IntSet insert " << TEST_SIZE << " items took: " << duration << "ms\n";

    IntSet::destroy(is);
}

// Main test runner for intset
inline void run_intset_tests()
{
    try
    {
        std::cout << "Running basic operations tests...\n";
        test_intset_basic_ops();
        std::cout << "✓ Basic operations tests passed\n\n";

        std::cout << "Running encoding upgrade tests...\n";
        test_intset_upgrades();
        std::cout << "✓ Encoding upgrade tests passed\n\n";

        std::cout << "Running performance tests...\n";
        test_intset_performance();
        std::cout << "✓ Performance tests passed\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        throw;
    }
}