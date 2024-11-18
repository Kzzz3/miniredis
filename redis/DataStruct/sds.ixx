export module sds;

import std;

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
private:
	sds_type type_;
	char buf_[];

	sds() = delete;

	// 访问并操作 hdr 的私有方法
	template <typename Func, typename Ret = std::invoke_result_t<Func, sdshdr<std::uint64_t>*>>
	auto access_sdshdr(Func&& operation) -> Ret;

public:
	size_t length();
	size_t capacity();
	size_t available();

	static void destroy(sds*& s);
	static sds* create(const char* str, size_t len);
};
#pragma pack(pop)