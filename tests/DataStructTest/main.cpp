#include "intset_test.hpp"
#include "sds_test.hpp"
#include "ziplist_test.hpp"
#include "zset_test.hpp"

int main()
{
    sds_tests();
    assert(Allocator::get_current_allocated() == 0);
    
    ziplist_tests();
    assert(Allocator::get_current_allocated() == 0);

    intset_test();
    assert(Allocator::get_current_allocated() == 0);

    zset_test();
    assert(Allocator::get_current_allocated() == 0);
}