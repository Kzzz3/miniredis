#include "intset_test.hpp"
#include "sds_test.hpp"
#include "ziplist_test.hpp"
#include "zset_test.hpp"

// Run all data structure tests
int main() {
    std::cout << "\n=== Running Data Structure Tests ===\n\n";
    
    std::cout << "=== SDS Tests ===\n";
    run_sds_tests();
    assert(Allocator::get_current_allocated() == 0);
    std::cout << "\n";
    
    std::cout << "=== Ziplist Tests ===\n";
    run_ziplist_tests();
    assert(Allocator::get_current_allocated() == 0);
    std::cout << "\n";
    
    std::cout << "=== IntSet Tests ===\n";
    run_intset_tests();
    assert(Allocator::get_current_allocated() == 0);
    std::cout << "\n";
    
    std::cout << "=== ZSet Tests ===\n";
    run_zset_tests();
    assert(Allocator::get_current_allocated() == 0);
    std::cout << "\n";

    std::cout << "All tests completed successfully!\n";
    return 0;
}