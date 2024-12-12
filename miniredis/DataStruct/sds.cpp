#include "sds.h"

void Sds::destroy(Sds* s)
{
    access_sdshdr(s,
                  [](auto psdshdr) -> void
                  {
                      Allocator::destroy_with_extra<remove_pointer_t<decltype(psdshdr)>>(
                          psdshdr, psdshdr->alloc);
                  });
}

Sds* Sds::create(Sds* str, size_t alloc)
{
    return create(str->buf, str->length(), alloc);
}

Sds* Sds::create(const char* str, size_t len, size_t alloc)
{
    alloc = max(alloc, len);

    auto allocate = [str, len, alloc]<typename T>() -> Sds*
    {
        SdsHdr<T>* hdr = Allocator::create_with_extra<SdsHdr<T>>(alloc);

        hdr->len = len;
        hdr->alloc = alloc;
        hdr->flags = sizeof(T);
        memcpy(hdr->buf, str, len);
        hdr->buf[len] = '\0';

        return reinterpret_cast<Sds*>(hdr->buf);
    };

    if (alloc < numeric_limits<uint8_t>::max())
    {
        return allocate.operator()<uint8_t>();
    }
    else if (alloc < numeric_limits<uint16_t>::max())
    {
        return allocate.operator()<uint16_t>();
    }
    else if (alloc < numeric_limits<uint32_t>::max())
    {
        return allocate.operator()<uint32_t>();
    }
    else
    {
        return allocate.operator()<uint64_t>();
    }
}

size_t Sds::length()
{
    return access_sdshdr(this, [](auto psdshdr) -> size_t { return psdshdr->len; });
}

size_t Sds::capacity()
{
    return access_sdshdr(this, [](auto psdshdr) -> size_t { return psdshdr->alloc; });
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
    return access_sdshdr(this, [](auto psdshdr) -> size_t
                         { return sizeof(remove_pointer_t<decltype(psdshdr)>) - 1; });
}

Sds* Sds::dilatation(size_t add_len)
{
    size_t len = length();
    size_t new_alloc = capacity() + add_len;

    if (new_alloc <= SDS_MAX_PREALLOC)
        new_alloc *= 2;
    else
        new_alloc += SDS_MAX_PREALLOC;

    auto allocate = [this, len, new_alloc]<typename T>() -> Sds*
    {
        return access_sdshdr(
            this,
            [len, new_alloc](auto psdshdr) -> Sds*
            {
                SdsHdr<T>* new_psdshdr =
                    Allocator::recreate_with_extra<SdsHdr<T>>(psdshdr, psdshdr->alloc, new_alloc);

                size_t offset = sizeof(SdsHdr<T>) - sizeof(remove_pointer_t<decltype(psdshdr)>);
                memmove(new_psdshdr->buf, new_psdshdr->buf - offset, len);

                new_psdshdr->alloc = new_alloc;
                new_psdshdr->len = len;
                new_psdshdr->flags = sizeof(T);
                new_psdshdr->buf[len] = '\0';

                return reinterpret_cast<Sds*>(new_psdshdr->buf);
            });
    };

    if (new_alloc < numeric_limits<uint8_t>::max())
    {
        return allocate.operator()<uint8_t>();
    }
    else if (new_alloc < numeric_limits<uint16_t>::max())
    {
        return allocate.operator()<uint16_t>();
    }
    else if (new_alloc < numeric_limits<uint32_t>::max())
    {
        return allocate.operator()<uint32_t>();
    }
    else
    {
        return allocate.operator()<uint64_t>();
    }
}

Sds* Sds::copy(Sds* str)
{
    return copy(str->buf, str->length());
}

Sds* Sds::copy(const char* str, size_t len)
{
    Sds* ret = len <= capacity() ? this : dilatation(len - capacity());

    memcpy(ret->buf, str, len);
    access_sdshdr(ret,
                  [len](auto psdshdr)
                  {
                      psdshdr->len = len;
                      psdshdr->buf[psdshdr->len] = '\0';
                  });
    return ret;
}

Sds* Sds::append(Sds* str)
{
    return append(str->buf, str->length());
}

Sds* Sds::append(const char* str, size_t len)
{
    Sds* ret = len <= available() ? this : dilatation(len - available());

    memcpy(ret->buf + ret->length(), str, len);
    access_sdshdr(ret,
                  [len](auto psdshdr)
                  {
                      psdshdr->len += len;
                      psdshdr->buf[psdshdr->len] = '\0';
                  });
    return ret;
}