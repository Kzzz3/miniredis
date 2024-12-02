module intset;

import std;
import <cassert>;

std::int64_t intset::get(std::uint32_t index)
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

std::uint32_t intset::search(std::int64_t value)
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

intset* intset::create()
{
	intset* is = reinterpret_cast<intset*>(std::malloc(sizeof(intset)));
	is->encoding = INTSET_ENC_INT16;
	is->length = 0;
	return is;
}

void intset::destroy(intset* is)
{
	std::free(is);
}

bool intset::contains(std::int64_t value)
{
	std::uint32_t index = search(value);
	return index != length && get(index) == value;
}

intset* intset::insert(std::int64_t value)
{
	std::uint32_t value_encoding = INTSET_ENC_INT16;
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

	std::uint32_t index = search(value);

	intset* new_is = resize(this, length + 1);
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

intset* intset::remove(std::int64_t value)
{
	std::uint32_t index = search(value);
	if (index == length || get(index) != value) return this;

	std::memmove(content + index * encoding, content + (index + 1) * encoding, (length - index - 1) * encoding);
	intset* new_is = resize(this, length - 1);
	return new_is;
}

intset* resize(intset* is, size_t size)
{
	intset* new_is = reinterpret_cast<intset*>(std::realloc(is, sizeof(intset) + size * is->encoding));
	new_is->length = size;
	return new_is;
}

intset* upgrade(intset* is)
{
	assert(is->encoding <= INTSET_ENC_INT64);

	std::uint32_t new_encoding = is->encoding * 2;
	intset* new_is = reinterpret_cast<intset*>(std::realloc(is, sizeof(intset) + is->length * new_encoding));

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
