export module intset;

import std;

export constexpr std::uint32_t INTSET_ENC_INT16 = sizeof(std::int16_t);
export constexpr std::uint32_t INTSET_ENC_INT32 = sizeof(std::int32_t);
export constexpr std::uint32_t INTSET_ENC_INT64 = sizeof(std::int64_t);

export class intset
{
public:
	std::uint32_t encoding;
	std::uint32_t length;
	std::byte content[];

public:
	std::int64_t get(std::uint32_t index);
	std::uint32_t search(std::int64_t value);

public:
	static intset* create();
	static void destroy(intset* is);

	bool contains(std::int64_t value);
	intset* insert(std::int64_t value);
	intset* remove(std::int64_t value);
};


export intset* upgrade(intset* is);
export intset* resize(intset* is, size_t size);