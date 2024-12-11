#pragma once 
#include <string>
#include <memory>
#include <cstdint>
#include <cstring>

#include "utility.hpp"

constexpr uint8_t ZIPLIST_END = 0xff;
constexpr uint8_t ZIPLIST_INT_64B = 0xe0;
constexpr uint8_t ZIPLIST_STR_06B = 0x00;
constexpr uint8_t ZIPLIST_STR_14B = 0x40;
constexpr uint8_t ZIPLIST_STR_32B = 0x80;
constexpr uint8_t ZIPLIST_PREVLEN_FIRST = 0xfe;
constexpr uint8_t ZIPLIST_ENCODING_HEADER = 0xc0;

// assitance struct for ziplist entry encoding and decoding
struct ZlEntry
{
	uint32_t prevrawlensize = 0;
	uint32_t prevrawlen = 0;
	uint32_t lensize = 0;
	uint32_t len = 0;
	uint8_t encoding = 0;
	union
	{
		uint8_t* ptr;
		int64_t num;
	} data;
};

#pragma pack(push, 1)
class ZipList
{
public:
	uint32_t total_bytes;
	uint32_t last_offset;
	uint16_t items_num;
	uint8_t buf[1] = { ZIPLIST_END };

public:
	static ZipList* create();
	static void destroy(ZipList* zl);

	ZipList* pop_back();
	ZipList* pop_front();

	uint8_t* index(int index);
	uint8_t* next(uint8_t* p);
	uint8_t* prev(uint8_t* p);

	ZipList* push_back(uint8_t* str, size_t len);
	ZipList* push_front(uint8_t* str, size_t len);
	ZipList* insert(uint8_t* p, uint8_t* str, size_t len);
};
#pragma pack(pop)

// internal functions
void strEncode(uint8_t* str, size_t len, ZlEntry& entry);
bool tryIntEncode(uint8_t* str, size_t len, ZlEntry& entry);
ZipList* adjustSubsequentNodes(ZipList* zl, size_t offset, int diff);


// outside functions
void entryDecode(uint8_t* p, ZlEntry& entry);
void entryEncode(uint8_t* p, ZlEntry& entry);
constexpr bool isStr(uint8_t encoding) { return (encoding & 0xC0) < 0xC0; }
