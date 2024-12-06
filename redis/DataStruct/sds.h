#pragma once
#include <limits>
#include <cassert>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "../utility.hpp"

using std::optional;

constexpr uint8_t SDS_TYPE_8 = sizeof(uint8_t);
constexpr uint8_t SDS_TYPE_16 = sizeof(uint16_t);
constexpr uint8_t SDS_TYPE_32 = sizeof(uint32_t);
constexpr uint8_t SDS_TYPE_64 = sizeof(uint64_t);

constexpr size_t SDS_MAX_PREALLOC = 1024 * 1024;

#pragma pack(push, 1)
template <typename T>
	requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
			 std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
struct SdsHdr {
	T len;
	T alloc;
	uint8_t flags;
	char buf[1] = { '\0' };
};
#pragma pack(pop)

#pragma pack(push, 1)
class Sds {
public:
	char buf[1] = { '\0' };

public:
	static void destroy(Sds* s);

	static Sds* create(Sds* str, size_t alloc = 0);
	static Sds* create(const char* str, size_t len, size_t alloc = 0);

	size_t length();
	size_t capacity();
	size_t available();
	size_t headersize();

	Sds* dilatation(size_t add_len);

	Sds* copy(Sds* str);
	Sds* copy(const char* str, size_t len);

	Sds* append(Sds* str);
	Sds* append(const char* str, size_t len);
};
#pragma pack(pop)

template <typename T>
constexpr optional<int64_t> sds2num(Sds* str)
{
	return str2num<T>(str->buf, str->length());
}

template <typename T>
constexpr Sds* num2sds(T num)
{
	std::string str = std::to_string(num);
	return Sds::create(str.c_str(), str.length(), str.length());
}

template <typename Func, typename Ret = std::invoke_result_t<Func, SdsHdr<uint64_t>*>>
constexpr auto access_sdshdr(Sds* str, Func&& operation) -> Ret;

namespace std {
	template <>
	struct hash<Sds*> {
		size_t operator()(const Sds* const& p) const {
			return std::hash<std::string_view>()(std::string_view(p->buf, const_cast<Sds*>(p)->length()));
		}
	};

	template <>
	struct equal_to<Sds*> {
		bool operator()(const Sds* const& lhs, const Sds* const& rhs) const {
			return std::string_view(lhs->buf, const_cast<Sds*>(lhs)->length()) ==
				std::string_view(rhs->buf, const_cast<Sds*>(rhs)->length());
		}
	};
}