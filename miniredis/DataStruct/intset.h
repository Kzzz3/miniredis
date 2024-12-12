#pragma once 
#include <limits>
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>

#include "Utility/allocator.hpp"

using std::byte;

constexpr uint32_t INTSET_ENC_INT16 = sizeof(int16_t);
constexpr uint32_t INTSET_ENC_INT32 = sizeof(int32_t);
constexpr uint32_t INTSET_ENC_INT64 = sizeof(int64_t);

class IntSet
{
public:
	uint32_t encoding;
	uint32_t length;
	byte content[];

public:
	int64_t get(uint32_t index);
	uint32_t search(int64_t value);

public:
	static IntSet* create();
	static void destroy(IntSet* is);

	bool contains(int64_t value);
	IntSet* insert(int64_t value);
	IntSet* remove(int64_t value);
};

IntSet* upgrade(IntSet* is);
IntSet* resize(IntSet* is, size_t size);