#include "intset.h"

std::int64_t IntSet::get(uint32_t index)
{
	switch (encoding)
	{
	case INTSET_ENC_INT16:
		return reinterpret_cast<std::int16_t*>(content)[index];
	case INTSET_ENC_INT32:
		return reinterpret_cast<std::int32_t*>(content)[index];
	case INTSET_ENC_INT64:
		return reinterpret_cast<std::int64_t*>(content)[index];
	default:
		break;
	}
}

uint32_t IntSet::search(std::int64_t value)
{
	std::int64_t middle_value = 0;
	std::int64_t middle = 0, left = 0, right = length;
	while (left<right)
	{
		middle = (left + right) / 2;

		middle_value = get(middle);
		if (middle_value == value)
		{
			return middle;
		}
		else if (middle_value < value)
		{
			left = middle + 1;
		}
		else
		{
			right = middle - 1;
		}
	}

	return left;
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

bool IntSet::contains(std::int64_t value)
{
	uint32_t index = search(value);
	return index != length && get(index) == value;
}

IntSet* IntSet::insert(std::int64_t value)
{
	uint32_t value_encoding = INTSET_ENC_INT16;
	if (value > std::numeric_limits<std::int32_t>::max()||value<std::numeric_limits<std::int32_t>::min())
	{
		value_encoding = INTSET_ENC_INT64;
	}
	else if (value>std::numeric_limits<std::int16_t>::max() || value < std::numeric_limits<std::int16_t>::min())
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
		reinterpret_cast<std::int16_t*>(new_is->content)[index] = value;
		break;
	case INTSET_ENC_INT32:
		reinterpret_cast<std::int32_t*>(new_is->content)[index] = value;
		break;
	case INTSET_ENC_INT64:
		reinterpret_cast<std::int64_t*>(new_is->content)[index] = value;
		break;
	default:
		assert(false);
	}

	return new_is;
}

IntSet* IntSet::remove(std::int64_t value)
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
	assert(is->encoding <= INTSET_ENC_INT64);

	uint32_t new_encoding = is->encoding * 2;
	IntSet* new_is = reinterpret_cast<IntSet*>(std::realloc(is, sizeof(IntSet) + is->length * new_encoding));

	switch (new_is->encoding)
	{
	case INTSET_ENC_INT16:
		for (std::int64_t i = new_is->length-1; i >=0; --i)
		{
			reinterpret_cast<std::int32_t*>(new_is->content)[i] = reinterpret_cast<std::int16_t*>(new_is->content)[i];
		}
		break;
	case INTSET_ENC_INT32:
		for (std::int64_t i = new_is->length - 1; i >= 0; --i)
		{
			reinterpret_cast<std::int64_t*>(new_is->content)[i] = reinterpret_cast<std::int32_t*>(new_is->content)[i];
		}
		break;
	default:
		assert(false);
	}

	new_is->encoding = new_encoding;
	return new_is;
}
