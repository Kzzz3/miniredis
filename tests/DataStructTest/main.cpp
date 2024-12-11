#pragma once
#include "sds_test.hpp"
#include "zset_test.hpp"
#include "ziplist_test.hpp"
#include "intset_test.hpp"


int main()
{
	sds_tests();
	ziplist_tests();
	intset_test();
	zset_test();
}