#include "sds.h"

void Sds::destroy(Sds* s)
{
    if (s == nullptr)
        return;

    size_t alloc = s->capacity();
    switch (static_cast<uint8_t>(s->buf[-1]))
    {
    case SDS_TYPE_8:
        Allocator::destroy_with_extra(SDS_HDR(s, uint8_t), alloc);
        break;
    case SDS_TYPE_16:
        Allocator::destroy_with_extra(SDS_HDR(s, uint16_t), alloc);
        break;
    case SDS_TYPE_32:
        Allocator::destroy_with_extra(SDS_HDR(s, uint32_t), alloc);
        break;
    case SDS_TYPE_64:
        Allocator::destroy_with_extra(SDS_HDR(s, uint64_t), alloc);
        break;
    default:
        assert(false);
    }
}

Sds* Sds::create(Sds* str, size_t alloc)
{
    return create(str->buf, str->length(), alloc);
}

Sds* Sds::create(const char* str, size_t len, size_t alloc)
{
    if (len == 0)
        len = strlen(str);
    alloc = max(alloc, len);

    static constexpr auto allocate = []<typename T>(const char* str, size_t len,
                                                    size_t alloc) -> Sds*
    {
        SdsHdr<T>* hdr = Allocator::create_with_extra<SdsHdr<T>>(alloc, len, alloc, sizeof(T));
        memcpy(hdr->buf, str, len);
        hdr->buf[len] = '\0';

        return reinterpret_cast<Sds*>(hdr->buf);
    };

    if (alloc < numeric_limits<uint8_t>::max())
    {
        return allocate.operator()<uint8_t>(str, len, alloc);
    }
    else if (alloc < numeric_limits<uint16_t>::max())
    {
        return allocate.operator()<uint16_t>(str, len, alloc);
    }
    else if (alloc < numeric_limits<uint32_t>::max())
    {
        return allocate.operator()<uint32_t>(str, len, alloc);
    }
    else
    {
        return allocate.operator()<uint64_t>(str, len, alloc);
    }
}

vector<char> Sds::serialize(Sds* str)
{
    std::string_view sv(str->buf, str->length());
    return struct_pack::serialize(sv);
}

void Sds::serialize_to(ofstream& ofs, Sds* str)
{
    std::string_view sv(str->buf, str->length());
    struct_pack::serialize_to(ofs, sv);
}

void Sds::serialize_to(vector<char>& vec, Sds* str)
{
    std::string_view sv(str->buf, str->length());
    struct_pack::serialize_to(vec, sv);
}

Sds* Sds::deserialize_from(ifstream& ifs)
{
    auto sv = struct_pack::deserialize<string>(ifs);
    if (sv.has_value())
    {
        return create(sv.value().data(), sv.value().size());
    }
    else
    {
        throw std::runtime_error("deserialize failed");
    }
}

Sds* Sds::deserialize_from(const vector<char>& vec)
{
    auto sv = struct_pack::deserialize<string>(vec);
    if (sv.has_value())
    {
        return create(sv.value().data(), sv.value().size());
    }
    else
    {
        throw std::runtime_error("deserialize failed");
    }
}

size_t Sds::length()
{
    switch (static_cast<uint8_t>(this->buf[-1]))
    {
    case SDS_TYPE_8:
        return SDS_HDR(this, uint8_t)->len;
    case SDS_TYPE_16:
        return SDS_HDR(this, uint16_t)->len;
    case SDS_TYPE_32:
        return SDS_HDR(this, uint32_t)->len;
    case SDS_TYPE_64:
        return SDS_HDR(this, uint64_t)->len;
    default:
        assert(false);
    }
}

size_t Sds::capacity()
{
    switch (static_cast<uint8_t>(this->buf[-1]))
    {
    case SDS_TYPE_8:
        return SDS_HDR(this, uint8_t)->alloc;
    case SDS_TYPE_16:
        return SDS_HDR(this, uint16_t)->alloc;
    case SDS_TYPE_32:
        return SDS_HDR(this, uint32_t)->alloc;
    case SDS_TYPE_64:
        return SDS_HDR(this, uint64_t)->alloc;
    default:
        assert(false);
    }
}

size_t Sds::available()
{
    return capacity() - length();
}

size_t Sds::totalsize()
{
    return headersize() + length() + 1;
}

size_t Sds::headersize()
{
    switch (static_cast<uint8_t>(this->buf[-1]))
    {
    case SDS_TYPE_8:
        return sizeof(SdsHdr<uint8_t>) - 1;
    case SDS_TYPE_16:
        return sizeof(SdsHdr<uint16_t>) - 1;
    case SDS_TYPE_32:
        return sizeof(SdsHdr<uint32_t>) - 1;
    case SDS_TYPE_64:
        return sizeof(SdsHdr<uint64_t>) - 1;
    default:
        assert(false);
    }
}

Sds* Sds::dilatation(size_t add_len)
{
    if (add_len == 0)
        return this;
    size_t new_alloc = capacity() + add_len;

    if (new_alloc <= SDS_MAX_PREALLOC)
        new_alloc *= 2;
    else
        new_alloc += SDS_MAX_PREALLOC;

    static constexpr auto allocate = []<typename T_NEW>(Sds* str, size_t new_alloc) -> Sds*
    {
        size_t len = str->length();
        size_t alloc = str->capacity();
        size_t old_sdshdr_size = 0;
        SdsHdr<T_NEW>* new_psdshdr = nullptr;
        switch (static_cast<uint8_t>(str->buf[-1]))
        {
        case SDS_TYPE_8:
            old_sdshdr_size = sizeof(SdsHdr<uint8_t>);
            new_psdshdr = Allocator::recreate_with_extra<SdsHdr<T_NEW>>(SDS_HDR(str, uint8_t),
                                                                        alloc, new_alloc);
            break;
        case SDS_TYPE_16:
            old_sdshdr_size = sizeof(SdsHdr<uint16_t>);
            new_psdshdr = Allocator::recreate_with_extra<SdsHdr<T_NEW>>(SDS_HDR(str, uint16_t),
                                                                        alloc, new_alloc);
            break;
        case SDS_TYPE_32:
            old_sdshdr_size = sizeof(SdsHdr<uint32_t>);
            new_psdshdr = Allocator::recreate_with_extra<SdsHdr<T_NEW>>(SDS_HDR(str, uint32_t),
                                                                        alloc, new_alloc);
            break;
        case SDS_TYPE_64:
            old_sdshdr_size = sizeof(SdsHdr<uint64_t>);
            new_psdshdr = Allocator::recreate_with_extra<SdsHdr<T_NEW>>(SDS_HDR(str, uint64_t),
                                                                        alloc, new_alloc);
            break;
        default:
            assert(false);
        }

        size_t offset = sizeof(SdsHdr<T_NEW>) - old_sdshdr_size;
        memmove(new_psdshdr->buf, new_psdshdr->buf - offset, len);

        new_psdshdr->alloc = new_alloc;
        new_psdshdr->len = len;
        new_psdshdr->flags = sizeof(T_NEW);
        new_psdshdr->buf[len] = '\0';

        return reinterpret_cast<Sds*>(new_psdshdr->buf);
    };

    if (new_alloc < numeric_limits<uint8_t>::max())
    {
        return allocate.operator()<uint8_t>(this, new_alloc);
    }
    else if (new_alloc < numeric_limits<uint16_t>::max())
    {
        return allocate.operator()<uint16_t>(this, new_alloc);
    }
    else if (new_alloc < numeric_limits<uint32_t>::max())
    {
        return allocate.operator()<uint32_t>(this, new_alloc);
    }
    else
    {
        return allocate.operator()<uint64_t>(this, new_alloc);
    }
}

Sds* Sds::copy(Sds* str)
{
    return copy(str->buf, str->length());
}

Sds* Sds::copy(const char* str, size_t len)
{
    size_t avail = capacity();
    Sds* ret = len <= avail ? this : dilatation(len - avail);

    memcpy(ret->buf, str, len);
    switch (static_cast<uint8_t>(ret->buf[-1]))
    {
    case SDS_TYPE_8:
        SDS_HDR(ret, uint8_t)->len = len;
        SDS_HDR(ret, uint8_t)->buf[len] = '\0';
        break;
    case SDS_TYPE_16:
        SDS_HDR(ret, uint16_t)->len = len;
        SDS_HDR(ret, uint16_t)->buf[len] = '\0';
        break;
    case SDS_TYPE_32:
        SDS_HDR(ret, uint32_t)->len = len;
        SDS_HDR(ret, uint32_t)->buf[len] = '\0';
        break;
    case SDS_TYPE_64:
        SDS_HDR(ret, uint64_t)->len = len;
        SDS_HDR(ret, uint64_t)->buf[len] = '\0';
        break;
    default:
        assert(false);
    }

    return ret;
}

Sds* Sds::append(Sds* str)
{
    return append(str->buf, str->length());
}

Sds* Sds::append(const char* str, size_t len)
{
    size_t avail = available();
    Sds* ret = len <= avail ? this : dilatation(len - avail);

    memcpy(ret->buf + ret->length(), str, len);
    switch (static_cast<uint8_t>(ret->buf[-1]))
    {
    case SDS_TYPE_8:
        SDS_HDR(ret, uint8_t)->len += len;
        SDS_HDR(ret, uint8_t)->buf[SDS_HDR(ret, uint8_t)->len] = '\0';
        break;
    case SDS_TYPE_16:
        SDS_HDR(ret, uint16_t)->len += len;
        SDS_HDR(ret, uint16_t)->buf[SDS_HDR(ret, uint16_t)->len] = '\0';
        break;
    case SDS_TYPE_32:
        SDS_HDR(ret, uint32_t)->len += len;
        SDS_HDR(ret, uint32_t)->buf[SDS_HDR(ret, uint32_t)->len] = '\0';
        break;
    case SDS_TYPE_64:
        SDS_HDR(ret, uint64_t)->len += len;
        SDS_HDR(ret, uint64_t)->buf[SDS_HDR(ret, uint64_t)->len] = '\0';
        break;
    default:
        assert(false);
    }

    // access_sdshdr(ret,
    //               [len](auto psdshdr)
    //               {
    //                   psdshdr->len += len;
    //                   psdshdr->buf[psdshdr->len] = '\0';
    //               });
    return ret;
}

int Sds::strcmp(Sds* str)
{
    return Sds::strcmp(str->buf, str->length());
}

int Sds::strcmp(const char* str)
{
    return Sds::strcmp(str, strlen(str));
}

int Sds::strcmp(const char* str, size_t len)
{
    return string_view(buf, length()).compare(string_view(str, len));
}

void Sds::convertToLower()
{
    int len = length();
    for (int i = 0; i < len; i++)
    {
        buf[i] = tolower(buf[i]);
    }
}

void Sds::convertToUpper()
{
    int len = length();
    for (int i = 0; i < len; i++)
    {
        buf[i] = toupper(buf[i]);
    }
}