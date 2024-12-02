export module dict_test;

import dict;
import <cassert>;

export void dict_test()
{
	dict d;
	sds* key = sds::create("key", 3, 3);
	d[key].i64num = 1;
	sds* key1 = sds::create("key", 4, 4);
	d[key1].i64num = 2;

	std::cout << d[key].i64num << std::endl;
	std::cout << d[key1].i64num << std::endl;
	
	assert(d[key].i64num == 1);
	assert(d[key1].i64num == 2);
}