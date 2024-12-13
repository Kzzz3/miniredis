#pragma once
#include "DataStruct/ziplist.h"

// Utility functions for ziplist validation
inline void validate_ziplist(ZipList* zl) {
    uint8_t* p = zl->buf;
    for (uint16_t i = 0; i < zl->items_num; ++i) {
        ZlEntry entry;
        entryDecode(p, entry);
        if (i > 0) {
            ZlEntry prev_entry;
            entryDecode(p - entry.prevrawlen, prev_entry);
            assert(prev_entry.len + prev_entry.lensize + prev_entry.prevrawlensize == entry.prevrawlen);
        }
        p += entry.prevrawlensize + entry.lensize + entry.len;
    }
}

inline void print_ziplist_info(ZipList* zl) {
    std::cout << "Ziplist stats - Total bytes: " << zl->total_bytes 
              << ", Items: " << zl->items_num
              << ", Last offset: " << zl->last_offset << "\n";
}

// Test basic operations
inline void test_ziplist_basic_ops() {
    ZipList* zl = ZipList::create();

    // Test push operations
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("test1")), 5);
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("test2")), 5);
    validate_ziplist(zl);

    // Test pop operations
    zl = zl->pop_back();
    zl = zl->pop_front();
    validate_ziplist(zl);

    ZipList::destroy(zl);
}

// Test edge cases
inline void test_ziplist_edge_cases() {
    ZipList* zl = ZipList::create();

    // Test with empty string
    zl = zl->push_back(reinterpret_cast<uint8_t*>(const_cast<char*>("")), 0);
    validate_ziplist(zl);

    // Test with large string
    std::string large_str(16384, 'x');  // 16KB string
    zl = zl->push_back(reinterpret_cast<uint8_t*>(large_str.data()), large_str.size());
    validate_ziplist(zl);

    ZipList::destroy(zl);
}

// Test performance
inline void test_ziplist_performance() {
    ZipList* zl = ZipList::create();
    const int TEST_SIZE = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    
    // Insert test
    for (int i = 0; i < TEST_SIZE; ++i) {
        std::string data = "test" + std::to_string(i);
        zl = zl->push_back(reinterpret_cast<uint8_t*>(data.data()), data.size());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Ziplist insert " << TEST_SIZE << " items took: " << duration << "ms\n";

    ZipList::destroy(zl);
}

// Main test runner for ziplist
inline void run_ziplist_tests() {
    try {
        std::cout << "Running basic operations tests...\n";
        test_ziplist_basic_ops();
        std::cout << "✓ Basic operations tests passed\n\n";

        std::cout << "Running edge cases tests...\n";
        test_ziplist_edge_cases();
        std::cout << "✓ Edge cases tests passed\n\n";

        std::cout << "Running performance tests...\n";
        test_ziplist_performance();
        std::cout << "✓ Performance tests passed\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        throw;
    }
}