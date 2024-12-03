#pragma once

#include <cassert>
#include <iostream>

#include "../DataStruct/dict.h"

inline void dict_test()
{
	Dict d;
	Sds* key = Sds::create("key", 3, 3);
	d[key].i64num = 1;
	Sds* key1 = Sds::create("key", 4, 4);
	d[key1].i64num = 2;

	std::cout << d[key].i64num << std::endl;
	std::cout << d[key1].i64num << std::endl;
	
	assert(d[key].i64num == 1);
	assert(d[key1].i64num == 2);
}