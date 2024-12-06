#include "intset.h"

int64_t IntSet::get(uint32_t index)
{
	switch (encoding)
	{
	case INTSET_ENC_INT16:
		return reinterpret_cast<int16_t*>(content)[index];
	case INTSET_ENC_INT32:
		return reinterpret_cast<int32_t*>(content)[index];
	case INTSET_ENC_INT64:
		return reinterpret_cast<int64_t*>(content)[index];
	default:
		assert(false);
	}
}

uint32_t IntSet::search(int64_t value)
{
	constexpr auto searchImpl = []<typename T>(byte * content, uint32_t length, int64_t value) -> uint32_t {
		T* start = reinterpret_cast<T*>(content);
		return std::lower_bound(start, start + length, static_cast<T>(value)) - start;
	};

	switch (encoding) 
	{
	case INTSET_ENC_INT16:
		return searchImpl.operator() < int16_t > (content, length, value);
	case INTSET_ENC_INT32:
		return searchImpl.operator() < int32_t > (content, length, value);
	case INTSET_ENC_INT64:
		return searchImpl.operator() < int64_t > (content, length, value);
	default:
		assert(false);
	}
}

IntSet* IntSet::create()
{
	IntSet* is = reinterpret_cast<IntSet*>(std::malloc(sizeof(IntSet)));
	is->encoding = INTSET_ENC_INT16;
	is->length = 0;
	return is;
}

void IntSet::destroy(IntSet* is)
{
	std::free(is);
}

bool IntSet::contains(int64_t value)
{
	uint32_t index = search(value);
	return index != length && get(index) == value;
}

IntSet* IntSet::insert(int64_t value)
{
	uint32_t value_encoding = INTSET_ENC_INT16;
	if (value > std::numeric_limits<int32_t>::max() || value < std::numeric_limits<int32_t>::min())
	{
		value_encoding = INTSET_ENC_INT64;
	}
	else if (value > std::numeric_limits<int16_t>::max() || value < std::numeric_limits<int16_t>::min())
	{
		value_encoding = INTSET_ENC_INT32;
	}
	if (value_encoding > encoding)
	{
		return upgrade(this)->insert(value);
	}

	uint32_t index = search(value);
	IntSet* new_is = resize(this, length + 1);
	std::memmove(new_is->content + (index + 1) * encoding, new_is->content + index * encoding, (length - index) * encoding);

	switch (encoding)
	{
	case INTSET_ENC_INT16:
		reinterpret_cast<int16_t*>(new_is->content)[index] = value;
		break;
	case INTSET_ENC_INT32:
		reinterpret_cast<int32_t*>(new_is->content)[index] = value;
		break;
	case INTSET_ENC_INT64:
		reinterpret_cast<int64_t*>(new_is->content)[index] = value;
		break;
	default:
		assert(false);
	}
	return new_is;
}

IntSet* IntSet::remove(int64_t value)
{
	uint32_t index = search(value);
	if (index == length || get(index) != value) return this;

	std::memmove(content + index * encoding, content + (index + 1) * encoding, (length - index - 1) * encoding);
	IntSet* new_is = resize(this, length - 1);
	return new_is;
}

IntSet* resize(IntSet* is, size_t size)
{
	IntSet* new_is = reinterpret_cast<IntSet*>(std::realloc(is, sizeof(IntSet) + size * is->encoding));
	new_is->length = size;
	return new_is;
}

IntSet* upgrade(IntSet* is)
{
	if (is->encoding == INTSET_ENC_INT64)
		assert(false);

	uint32_t new_encoding = is->encoding * 2;
	IntSet* new_is = reinterpret_cast<IntSet*>(std::realloc(is, sizeof(IntSet) + is->length * new_encoding));

	switch (new_is->encoding)
	{
	case INTSET_ENC_INT16:
		for (int64_t i = new_is->length - 1; i >= 0; --i)
		{
			reinterpret_cast<int32_t*>(new_is->content)[i] = reinterpret_cast<int16_t*>(new_is->content)[i];
		}
		break;
	case INTSET_ENC_INT32:
		for (int64_t i = new_is->length - 1; i >= 0; --i)
		{
			reinterpret_cast<int64_t*>(new_is->content)[i] = reinterpret_cast<int32_t*>(new_is->content)[i];
		}
		break;
	default:
		assert(false);
	}

	new_is->encoding = new_encoding;
	return new_is;
}
