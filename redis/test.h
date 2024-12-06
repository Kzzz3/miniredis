#pragma once
#include "Test/sds_test.h"
#include "Test/ziplist_test.h"
#include "Test/intset_test.h"
#include "Test/zset_test.hpp"

inline void test()
{
	run_sds_tests();
	run_ziplist_tests();
	intset_test();
	zset_test();
}