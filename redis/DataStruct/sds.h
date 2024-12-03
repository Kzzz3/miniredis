#pragma once
#include <limits>
#include <concepts>
#include <stdexcept>
#include <string_view>

constexpr size_t SDS_MAX_PREALLOC = 1024 * 1024;

enum class SdsType 
	: uint8_t {
	SDS_TYPE_8 = 1,
	SDS_TYPE_16 = 2,
	SDS_TYPE_32 = 4,
	SDS_TYPE_64 = 8,
};

#pragma pack(push, 1)
template <typename T>
	requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
			 std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
struct SdsHdr {
	T len;
	T alloc;
	SdsType type;
	char buf[1] = { '\0' };
};
#pragma pack(pop)

#pragma pack(push, 1)
class Sds {
public:
	SdsType type;
	char buf[1] = { '\0' };

public:
	static void destroy(Sds* s);
	static Sds* create(const char* str, size_t len, size_t alloc);

	size_t length();
	size_t capacity();
	size_t available();

	Sds* dilatation(size_t add_len);

	Sds* copy(Sds* str);
	Sds* copy(const char* str, size_t len);

	Sds* append(Sds* str);
	Sds* append(const char* str, size_t len);
};
#pragma pack(pop)

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