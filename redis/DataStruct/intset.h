#pragma once 
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstddef>

using std::byte;

constexpr uint32_t INTSET_ENC_INT16 = sizeof(std::int16_t);
constexpr uint32_t INTSET_ENC_INT32 = sizeof(std::int32_t);
constexpr uint32_t INTSET_ENC_INT64 = sizeof(std::int64_t);

class IntSet
{
public:
	uint32_t encoding;
	uint32_t length;
	byte content[];

public:
	std::int64_t get(uint32_t index);
	uint32_t search(std::int64_t value);

public:
	static IntSet* create();
	static void destroy(IntSet* is);

	bool contains(std::int64_t value);
	IntSet* insert(std::int64_t value);
	IntSet* remove(std::int64_t value);
};


IntSet* upgrade(IntSet* is);
IntSet* resize(IntSet* is, size_t size);