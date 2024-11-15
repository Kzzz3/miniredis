export module sds;

import std;

export enum class sds_type : std::uint8_t {
	SDS_TYPE_8,
	SDS_TYPE_16 = 1,
	SDS_TYPE_32 = 2,
	SDS_TYPE_64 = 3,
};

#pragma pack(push, 1)
export template <typename T>
	requires std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
		     std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>
struct sdshdr {
	T len;
	T alloc;
	sds_type type;

	char buf[0];
};
#pragma pack(pop)

export class sds {
private:
	char* buf_ = nullptr;

	template <typename Func>
	void access_hdr(Func&& operation);

public:
	 size_t length();
	 size_t capacity();
	 size_t available();

	void free();
	void init(const char* str, size_t len);
};