export module ziplist;

import std;

export constexpr std::uint8_t ZIPLIST_END = 0xff;
export constexpr std::uint8_t ZIPLIST_INT_64B = 0xe0;
export constexpr std::uint8_t ZIPLIST_STR_06B = 0x00;
export constexpr std::uint8_t ZIPLIST_STR_14B = 0x40;
export constexpr std::uint8_t ZIPLIST_STR_32B = 0x80;
export constexpr std::uint8_t ZIPLIST_PREVLEN_FIRST = 0xfe;
export constexpr std::uint8_t ZIPLIST_ENCODING_HEADER = 0xc0;

// assitance struct for ziplist entry encoding and decoding
export struct zlentry
{
	std::uint32_t prevrawlensize = 0;
	std::uint32_t prevrawlen = 0;
	std::uint32_t lensize = 0;
	std::uint32_t len = 0;
	std::uint8_t encoding = 0;
	union
	{
		std::uint64_t num;
		std::uint8_t* ptr;
	} data;
};

#pragma pack(push, 1)
export class ziplist
{
public:
	std::uint32_t total_bytes = 0;
	std::uint32_t last_offset = 0;
	std::uint16_t items_num = 0;
	std::uint8_t buf[1] = { ZIPLIST_END };

public:
	static ziplist* create();
	static void destroy(ziplist* zl);

	ziplist* pop_back();
	ziplist* pop_front();

	std::uint8_t* index(int index);
	std::uint8_t* next(std::uint8_t* p);
	std::uint8_t* prev(std::uint8_t* p);

	ziplist* push_back(std::uint8_t* str, size_t len);
	ziplist* push_front(std::uint8_t* str, size_t len);
	ziplist* insert(std::uint8_t* p, std::uint8_t* str, size_t len);
};
#pragma pack(pop)

// internal functions
void strEncode(std::uint8_t* str, size_t len, zlentry& entry);
bool tryIntEncode(std::uint8_t* str, size_t len, zlentry& entry);

ziplist* adjustSubsequentNodes(ziplist* zl, size_t offset, int diff);


// outside functions
export
{
	void entryDecode(std::uint8_t* p, zlentry& entry);
	void entryEncode(std::uint8_t* p, zlentry& entry);

	constexpr bool isStr(std::uint8_t encoding) { return (encoding & 0xC0) < 0xC0; }
}


