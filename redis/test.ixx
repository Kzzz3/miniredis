export module test;

import sds_test;
import ziplist_test;
import intset_test;
import dict_test;

export void test()
{
	run_sds_tests();
	run_ziplist_tests();
	intset_test();
	dict_test();
}