#pragma once
#include <limits>
#include <cassert>
#include <cstring>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "Utility/utility.hpp"
#include "Utility/allocator.hpp"

using std::hash;
using std::invoke_result_t;
using std::numeric_limits;
using std::optional;
using std::remove_pointer_t;
using std::string;
using std::string_view;

using std::max;
using std::to_string;

constexpr uint8_t SDS_TYPE_8 = sizeof(uint8_t);
constexpr uint8_t SDS_TYPE_16 = sizeof(uint16_t);
constexpr uint8_t SDS_TYPE_32 = sizeof(uint32_t);
constexpr uint8_t SDS_TYPE_64 = sizeof(uint64_t);

constexpr size_t SDS_MAX_PREALLOC = 1024 * 1024;

#pragma pack(push, 1)
template <typename T>
    requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
             std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
struct SdsHdr
{
    T len;
    T alloc;
    uint8_t flags;
    char buf[1] = {'\0'};
};
#pragma pack(pop)

#pragma pack(push, 1)
class Sds
{
public:
    char buf[1] = {'\0'};

public:
    static void destroy(Sds* s);

    static Sds* create(Sds* str, size_t alloc = 0);
    static Sds* create(const char* str, size_t len, size_t alloc = 0);

    size_t length();
    size_t capacity();
    size_t available();
    size_t totalsize();
    size_t headersize();

    Sds* dilatation(size_t add_len);

    Sds* copy(Sds* str);
    Sds* copy(const char* str, size_t len);

    Sds* append(Sds* str);
    Sds* append(const char* str, size_t len);
};
#pragma pack(pop)

template <typename T> constexpr optional<int64_t> sds2num(Sds* str)
{
    return str2num<T>(str->buf, str->length());
}

template <typename T> constexpr Sds* num2sds(T num)
{
    string str = to_string(num);
    return Sds::create(str.c_str(), str.length(), str.length());
}

template <typename Func, typename Ret = invoke_result_t<Func, SdsHdr<uint64_t>*>>
static constexpr auto access_sdshdr(Sds* str, Func&& operation) -> Ret
{
    uint8_t flags = reinterpret_cast<uint8_t*>(str)[-1];
    switch (flags)
    {
    case SDS_TYPE_8:
        return operation(
            reinterpret_cast<SdsHdr<uint8_t>*>(str->buf - sizeof(SdsHdr<uint8_t>) + 1));
    case SDS_TYPE_16:
        return operation(
            reinterpret_cast<SdsHdr<uint16_t>*>(str->buf - sizeof(SdsHdr<uint16_t>) + 1));
    case SDS_TYPE_32:
        return operation(
            reinterpret_cast<SdsHdr<uint32_t>*>(str->buf - sizeof(SdsHdr<uint32_t>) + 1));
    case SDS_TYPE_64:
        return operation(
            reinterpret_cast<SdsHdr<uint64_t>*>(str->buf - sizeof(SdsHdr<uint64_t>) + 1));
    default:
        assert(false);
    }
}

namespace std
{
template <> struct hash<Sds*>
{
    size_t operator()(const Sds* const& p) const
    {
        return hash<string_view>()(string_view(p->buf, const_cast<Sds*>(p)->length()));
    }
};

template <> struct equal_to<Sds*>
{
    bool operator()(const Sds* const& lhs, const Sds* const& rhs) const
    {
        return string_view(lhs->buf, const_cast<Sds*>(lhs)->length()) ==
               string_view(rhs->buf, const_cast<Sds*>(rhs)->length());
    }
};
} // namespace std