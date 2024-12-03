#pragma once
#include "Test/sds_test.h"
#include "Test/ziplist_test.h"
#include "Test/intset_test.h"
#include "Test/dict_test.h"

inline void test()
{
	run_sds_tests();
	run_ziplist_tests();
	intset_test();
	dict_test();
}