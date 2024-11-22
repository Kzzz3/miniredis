import ziplist;
import std;
import <cassert>;

void check(ziplist* zl)
{
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

void printZiplist(ziplist* zl)
{
	std::cout << "Total bytes: " << zl->total_bytes << ", Items: " << zl->items_num << ", Last offset: " << zl->last_offset << "\n";

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
			std::cout << "data=" << std::string(reinterpret_cast<char*>(entry.data.ptr), entry.len) << "\n";
		}
		else {
			std::cout << "data=" << entry.data.num << "\n";
		}

		p += entry.prevrawlensize + entry.lensize + entry.len;
	}
}

void test1() {
	// Test 1: Create a new ziplist and add an integer
	ziplist* zl = new ziplist;
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("123")), 3);
	printZiplist(zl);

	// Test 2: Add a short string
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("hello")), 5);
	printZiplist(zl);

	// Test 3: Add a long string
	std::string longStr(1000, 'a');
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(longStr.data()), longStr.size());
	printZiplist(zl);

	// Test 4: Add multiple entries to test prevrawlen encoding
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("456")), 3);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("world")), 5);
	printZiplist(zl);

	// Test 5: Add a very long string to test edge case of lensize
	std::string veryLongStr(0x4000, 'b'); // 0x4000 = 16384 (just beyond 2-byte lensize limit)
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(veryLongStr.data()), veryLongStr.size());
	printZiplist(zl);

	// Test 6: Insert at the front
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(const_cast<char*>("front")), 5);
	printZiplist(zl);

	// Test 7: Insert at a specific position
	zl = zl->insert(zl->buf + zl->last_offset, reinterpret_cast<std::uint8_t*>(const_cast<char*>("middle")), 6);
	printZiplist(zl);

	// Test 8: Edge case for maximum possible entry size
	std::string maxStr(0xFFFFFF, 'z'); // Large string (almost max possible size in memory)
	try {
		zl = zl->push_back(reinterpret_cast<std::uint8_t*>(maxStr.data()), maxStr.size());
		printZiplist(zl);
	}
	catch (const std::bad_alloc&) {
		std::cout << "  Skipped max size test due to memory constraints.\n";
	}

	// Test 9: Add entries until memory is exhausted or large number of items
	try {
		for (size_t i = 0; i < 1000; ++i) {
			zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("bulk")), 4);
			if (i % 100 == 0) {
				std::cout << "Added " << i << " entries.\n";
			}
		}
	}
	catch (const std::bad_alloc&) {
		std::cout << "  Memory exhausted after adding large number of entries.\n";
	}
	printZiplist(zl);

	check(zl);

	::operator delete(zl);
	zl = nullptr;
}

void test10()
{
	// Test 10: Chain update test
	std::cout << "Test 10: Chain Update Test\n";
	ziplist* zl = new ziplist;

	// Add initial entries
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("short")), 5);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("medium_length")), 13);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("long_long_string_for_test")), 25);
	printZiplist(zl);

	// Insert a very long string at the second position, triggering chain updates
	std::string longerStr(1000, 'x');
	zl = zl->insert(zl->buf + zl->last_offset, reinterpret_cast<std::uint8_t*>(longerStr.data()), longerStr.size());
	std::cout << "After inserting a long string:\n";
	printZiplist(zl);

	// Modify the first entry to a larger size, forcing chain update
	zl = zl->insert(zl->buf, reinterpret_cast<std::uint8_t*>(const_cast<char*>("extra_long_entry")), 16);
	std::cout << "After modifying the first entry to a larger size:\n";
	printZiplist(zl);

	check(zl);

	::operator delete (zl);
	zl = nullptr;
}

void test11()
{
	// Test 11: Chain update test
	std::cout << "Test 11: Chain Update Test\n";
	ziplist* zl = new ziplist;

	std::string str1(250, 'x');
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(str1.data()), str1.size());
	printZiplist(zl);

	std::string str2(250, 'y');
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(str2.data()), str2.size());
	printZiplist(zl);

	std::string str3(250, 'z');
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(str3.data()), str3.size());
	printZiplist(zl);

	std::string str4(1000, 'z');
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(str4.data()), str4.size());
	printZiplist(zl);

	check(zl);

	::operator delete(zl);
	zl = nullptr;
}

void test12()
{
	ziplist* zl = new ziplist;
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("123")), 3);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("hello")), 5);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("456")), 3);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("world")), 5);
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(const_cast<char*>("front")), 5);
	zl = zl->insert(zl->buf + zl->last_offset, reinterpret_cast<std::uint8_t*>(const_cast<char*>("middle")), 6);
	printZiplist(zl);
	check(zl);

	zl = zl->pop_back();
	printZiplist(zl);
	check(zl);

	zl = zl->pop_front();
	printZiplist(zl);
	check(zl);
}

void test13()
{
	ziplist* zl = new ziplist;
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("123")), 3);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("hello")), 5);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("456")), 3);
	zl = zl->push_back(reinterpret_cast<std::uint8_t*>(const_cast<char*>("world")), 5);
	zl = zl->push_front(reinterpret_cast<std::uint8_t*>(const_cast<char*>("front")), 5);
	zl = zl->insert(zl->buf + zl->last_offset, reinterpret_cast<std::uint8_t*>(const_cast<char*>("middle")), 6);
	printZiplist(zl);
	check(zl);

	auto p = zl->index(3);
	zlentry entry;
	entryDecode(p, entry);
	std::cout << "Entry 3: prevrawlen=" << entry.prevrawlen << ", len=" << entry.len << ", encoding=" << std::hex << static_cast<int>(entry.encoding) << std::dec << "\n";

	p = zl->next(p);
	entryDecode(p, entry);
	std::cout << "Entry 4: prevrawlen=" << entry.prevrawlen << ", len=" << entry.len << ", encoding=" << std::hex << static_cast<int>(entry.encoding) << std::dec << "\n";

	p = zl->prev(p);
	entryDecode(p, entry);
	std::cout << "Entry 3: prevrawlen=" << entry.prevrawlen << ", len=" << entry.len << ", encoding=" << std::hex << static_cast<int>(entry.encoding) << std::dec << "\n";
}

int main() {
	std::cout << "Ziplist Test\n";
	test1();
	test10();
	test11();
	test12();
	test13();
	std::cout << "All tests passed.\n";
	return 0;
}