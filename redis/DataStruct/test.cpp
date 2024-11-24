import sds_test;
import ziplist_test;
import intset_test;

int main()
{
	run_sds_tests();
	run_ziplist_tests();
	intset_test();
}