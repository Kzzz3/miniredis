export module sds;

import std;

export constexpr size_t SDS_MAX_PREALLOC = 1024 * 1024;

export enum class sds_type : std::uint8_t {
	SDS_UNINIT,
	SDS_TYPE_8,
	SDS_TYPE_16,
	SDS_TYPE_32,
	SDS_TYPE_64,
};

#pragma pack(push, 1)
export template <typename T>
	requires std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
			 std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>
struct sdshdr {
	T len;
	T alloc;
	sds_type type;

	char buf[];
};
#pragma pack(pop)

#pragma pack(push, 1)
export class sds {
public:
	sds_type type;
	char buf[];

	template <typename Func, typename Ret = std::invoke_result_t<Func, sdshdr<std::uint64_t>*>>
	inline auto access_sdshdr(Func&& operation) -> Ret;

	size_t length();
	size_t capacity();
	size_t available();

	sds* dilatation(size_t add_len);
	sds* copy(sds* str);
	sds* copy(const char* str, size_t len);
	sds* append(sds* str);
	sds* append(const char* str, size_t len);


	static void destroy(sds* s);
	static sds* create(const char* str, size_t len, size_t alloc);
};
#pragma pack(pop)

template <typename Func, typename Ret>
inline auto sds::access_sdshdr(Func&& operation)-> Ret {
	switch (type) {
	case sds_type::SDS_TYPE_8:
		return operation(reinterpret_cast<sdshdr<std::uint8_t>*>((buf - sizeof(sdshdr<std::uint8_t>))));
	case sds_type::SDS_TYPE_16:
		return operation(reinterpret_cast<sdshdr<std::uint16_t>*>((buf - sizeof(sdshdr<std::uint16_t>))));
	case sds_type::SDS_TYPE_32:
		return operation(reinterpret_cast<sdshdr<std::uint32_t>*>((buf - sizeof(sdshdr<std::uint32_t>))));
	case sds_type::SDS_TYPE_64:
		return operation(reinterpret_cast<sdshdr<std::uint64_t>*>((buf - sizeof(sdshdr<std::uint64_t>))));
	default:
		throw std::runtime_error("Invalid SDS type.");
	}
}