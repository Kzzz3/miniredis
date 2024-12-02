export module ziplist_test;

import std;
import ziplist;
import <cassert>;

// Utility functions
namespace utils {

    void check(ziplist* zl) {
        std::uint8_t* p = zl->buf;
        for (std::uint16_t i = 0; i < zl->items_num; ++i) {
            zlentry entry;
            entryDecode(p, entry);
            if (i > 0) {
                zlentry prev_entry;
                entryDecode(p - entry.prevrawlen, prev_entry);
                assert(prev_entry.len + prev_entry.lensize + prev_entry.prevrawlensize == entry.prevrawlen);
            }
            p += entry.prevrawlensize + entry.lensize + entry.len;
        }
    }

    void printZiplist(ziplist* zl) {
        std::cout << "Total bytes: " << zl->total_bytes << ", Items: " << zl->items_num
            << ", Last offset: " << zl->last_offset << "\n";

        std::uint8_t* p = zl->buf;
        for (std::uint16_t i = 0; i < zl->items_num; ++i) {
            zlentry entry;
            entryDecode(p, entry);

            std::cout << "  Entry " << i + 1
                << ": prevrawlen=" << entry.prevrawlen
                << ", len=" << entry.len
                << ", encoding=" << std::hex << static_cast<int>(entry.encoding)
                << std::dec << ", ";

            if (isStr(entry.encoding)) {
                if (entry.len < 50)
					std::cout << "data=" << std::string(reinterpret_cast<char*>(entry.data.ptr), entry.len);
				std::cout << "\n";
            }
            else {
                std::cout << "data=" << entry.data.num << "\n";
            }

            p += entry.prevrawlensize + entry.lensize + entry.len;
        }
    }

    // Helper to check the contents of a single entry
    void check_entry(zlentry& entry, const std::string& expected_str) {
        if (isStr(entry.encoding)) {
            assert(std::string(reinterpret_cast<char*>(entry.data.ptr), entry.len) == expected_str);
        }
        else {
            std::cout << "Expected a string, but found other data type.\n";
        }
    }

    // Helper to validate that the ziplist is correctly formed
    void validate_ziplist_structure(ziplist* zl) {
        std::cout << "Validating ziplist structure...\n";
        assert(zl->total_bytes >= 0);
        assert(zl->items_num >= 0);
        check(zl);  // Check individual entries for consistency
    }
}  // namespace utils

// Test cases organized into individual modules
namespace tests {

    void testBasicOperations() {
        ziplist* zl = new ziplist;

        // Basic push operations
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("123")), 3);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("hello")), 5);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        std::string longStr(1000, 'a');
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(longStr.data()), longStr.size());
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        zl = zl->push_front(reinterpret_cast<std::uint8_t*>(const_cast<char*>("front")), 5);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        zl = zl->pop_back();
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        zl = zl->pop_front();
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        utils::check(zl);
        ::operator delete(zl);
    }

    void testEdgeCases() {
        ziplist* zl = new ziplist;

        // Adding large strings and edge-case operations
        std::string veryLongStr(0x4000, 'b');
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(veryLongStr.data()), veryLongStr.size());
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        try {
            std::string maxStr(0xFFFFFF, 'z');
            zl = zl->push_back(reinterpret_cast<std::uint8_t*>(maxStr.data()), maxStr.size());
            utils::printZiplist(zl);
        }
        catch (const std::bad_alloc&) {
            std::cout << "  Skipped max size test due to memory constraints.\n";
        }

        zl = zl->push_front(reinterpret_cast<std::uint8_t*>(const_cast<char*>("edge_case")), 9);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        utils::check(zl);
        ::operator delete(zl);
    }

    void testInsertAndChainUpdate() {
        ziplist* zl = new ziplist;

        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("short")), 5);
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("medium")), 6);
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("long_string")), 11);

        std::string longerStr(1000, 'x');
        zl = zl->insert(zl->buf + zl->last_offset, reinterpret_cast<std::uint8_t*>(longerStr.data()), longerStr.size());
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        zl = zl->insert(zl->buf, reinterpret_cast<std::uint8_t*>(const_cast<char*>("chain_update")), 12);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        utils::check(zl);
        ::operator delete(zl);
    }

    void testDeleteEdgeCases() {
        ziplist* zl = new ziplist;

        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("first")), 5);
        zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("second")), 6);
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        // Deleting all elements
        zl = zl->pop_back();
        zl = zl->pop_back();
        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        utils::check(zl);
        ::operator delete(zl);
    }

    void testMemoryManagement() {
        ziplist* zl = new ziplist;

        // Push and pop multiple times
        for (int i = 0; i < 10000; ++i) {
            std::string str = "item" + std::to_string(i);
            zl = zl->push_back(reinterpret_cast<std::uint8_t*>(str.data()), str.size());
        }

        // Pop items to ensure correct memory release
        while (zl->items_num > 0) {
            zl = zl->pop_back();
        }

        utils::printZiplist(zl);
        utils::validate_ziplist_structure(zl);

        utils::check(zl);
        ::operator delete(zl);
    }

    void testPerformance() {
        ziplist* zl = new ziplist;

        auto start = std::chrono::high_resolution_clock::now();
        // Insert 1 million elements for performance test
        for (int i = 0; i < 1000000; ++i) {
            std::string str = "item" + std::to_string(i);
            zl = zl->push_back(reinterpret_cast<std::uint8_t*>(str.data()), str.size());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Performance Test - Insertion time for 100w elements: " << diff.count() << " seconds.\n";

        start = std::chrono::high_resolution_clock::now();
        // Pop 1 million elements for performance test
        while (zl->items_num > 0) {
            zl = zl->pop_back();
        }
        end = std::chrono::high_resolution_clock::now();
        diff = end - start;
        std::cout << "Performance Test - Deletion time for 100w elements: " << diff.count() << " seconds.\n";

        utils::check(zl);
        ::operator delete(zl);
    }
}  // namespace tests

// External interface for running all tests
export void run_ziplist_tests() {
    std::cout << "Running ziplist tests...\n";

    tests::testBasicOperations();
    std::cout << "Test: Basic Operations passed.\n";

    tests::testEdgeCases();
    std::cout << "Test: Edge Cases passed.\n";

    tests::testInsertAndChainUpdate();
    std::cout << "Test: Insert and Chain Update passed.\n";

    tests::testDeleteEdgeCases();
    std::cout << "Test: Delete Edge Cases passed.\n";

    tests::testMemoryManagement();
    std::cout << "Test: Memory Management passed.\n";

    tests::testPerformance();
    std::cout << "Test: Performance passed.\n";
}
