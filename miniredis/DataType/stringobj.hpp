#pragma once
#include <fstream>
#include <optional>
#include <memory>
#include <cstdint>
#include <stdexcept>

#include "redisobj.h"
#include "DataStruct/sds.h"
#include "Utility/allocator.hpp"
#include "Utility/utility.hpp"

using std::ifstream;
using std::ofstream;
using std::optional;
using std::unique_ptr;

inline RedisObj* StringObjectCreate(Sds* str)
{
    RedisObj* obj = nullptr;
    size_t len = str->length();
    optional<int64_t> value = sds2num<int64_t>(str);

    if (value.has_value())
    {
        // integer
        obj = Allocator::create<RedisObj>();
        obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
        obj->data.num = value.value();
    }
    else if (len <= EMBSTR_MAX_LENGTH)
    {
        // embstr
        obj = Allocator::create_with_extra<RedisObj>(sizeof(SdsHdr<uint8_t>) + len);
        obj->encoding = ObjEncoding::REDIS_ENCODING_EMBSTR;

        SdsHdr<uint8_t>* hdr = reinterpret_cast<SdsHdr<uint8_t>*>(obj + 1);
        hdr->len = len;
        hdr->alloc = len;
        hdr->flags = SDS_TYPE_8;
        obj->data.ptr = hdr->buf;
        reinterpret_cast<Sds*>(obj->data.ptr)->copy(str);
    }
    else
    {
        // raw
        obj = Allocator::create<RedisObj>();
        obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
        obj->data.ptr = Sds::create(str);
    }

    obj->type = ObjType::REDIS_STRING;
    obj->lru = GetSecTimestamp();
    obj->refcount = 1;
    return obj;
}

inline RedisObj* StringObjectUpdate(RedisObj* obj, Sds* str)
{
    // Case 1: Encoding is INT, try to keep it as INT if possible
    if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
    {
        optional<int64_t> value = sds2num<int64_t>(str);
        if (value.has_value())
        {
            obj->data.num = value.value();
            return obj;
        }
    }

    // Case 2: Switch to EMBSTR if applicable
    int len = str->length();
    if (len <= EMBSTR_MAX_LENGTH && obj->encoding == ObjEncoding::REDIS_ENCODING_INT ||
        obj->encoding == ObjEncoding::REDIS_ENCODING_EMBSTR)
    {
        if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
        {
            obj = Allocator::recreate_with_extra<RedisObj>(obj, 0, sizeof(SdsHdr<uint8_t>) + len);
        }
        else
        {
            Sds* sds = reinterpret_cast<Sds*>(obj->data.ptr);
            obj = Allocator::recreate_with_extra<RedisObj>(
                obj, sizeof(SdsHdr<uint8_t>) + sds->length(), sizeof(SdsHdr<uint8_t>) + len);
        }
        obj->encoding = ObjEncoding::REDIS_ENCODING_EMBSTR;

        SdsHdr<uint8_t>* hdr = reinterpret_cast<SdsHdr<uint8_t>*>(obj + 1);
        hdr->len = len;
        hdr->alloc = len;
        hdr->flags = SDS_TYPE_8;
        obj->data.ptr = hdr->buf;
        reinterpret_cast<Sds*>(obj->data.ptr)->copy(str);

        return obj;
    }

    // Case 3: Switch to RAW encoding
    if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
    {
        obj->data.ptr = Sds::create(str);
    }
    else if (obj->encoding == ObjEncoding::REDIS_ENCODING_EMBSTR)
    {
        Sds* sds = reinterpret_cast<Sds*>(obj->data.ptr);
        obj = Allocator::recreate_with_extra<RedisObj>(obj, sizeof(SdsHdr<uint8_t>) + sds->length(),
                                                       0);
    }
    else
    {
        obj->data.ptr = reinterpret_cast<Sds*>(obj->data.ptr)->copy(str);
    }
    obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
    obj->lru = GetSecTimestamp();

    return obj;
}

inline auto StringObjectGet(RedisObj* obj)
{
    if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
    {
        return make_unique<ValueRef>(num2sds(obj->data.num), nullptr);
    }
    return make_unique<ValueRef>(reinterpret_cast<Sds*>(obj->data.ptr), obj);
}

inline void StringObjectDestroy(RedisObj* obj)
{
    if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
    {
        Allocator::destroy(obj);
    }
    else if (obj->encoding == ObjEncoding::REDIS_ENCODING_EMBSTR)
    {
        Sds* sds = reinterpret_cast<Sds*>(obj->data.ptr);
        Allocator::destroy_with_extra<RedisObj>(obj, sizeof(SdsHdr<uint8_t>) + sds->length());
    }
    else
    {
        Sds::destroy(reinterpret_cast<Sds*>(obj->data.ptr));
        Allocator::destroy(obj);
    }
}

inline void StringObjectDataSerialize(ofstream& ofs, RedisObj* obj)
{
    switch (obj->encoding)
    {
    case ObjEncoding::REDIS_ENCODING_INT:
        struct_pack::serialize_to(ofs, obj->data.num);
        break;
    case ObjEncoding::REDIS_ENCODING_EMBSTR:
        Sds::serialize_to(ofs, reinterpret_cast<Sds*>(obj->data.ptr));
        break;
    case ObjEncoding::REDIS_ENCODING_RAW:
        Sds::serialize_to(ofs, reinterpret_cast<Sds*>(obj->data.ptr));
        break;
    default:
        throw std::runtime_error("invalid encoding");
    }
}

inline void StringObjectDataDeserialize(ifstream& ifs, RedisObj* obj)
{
    struct_pack::expected<int64_t, struct_pack::err_code> expect_num;
    switch (obj->encoding)
    {
    case ObjEncoding::REDIS_ENCODING_INT:
        expect_num = struct_pack::deserialize<int64_t>(ifs);
        if (!expect_num.has_value())
            throw std::runtime_error("deserialize failed");
        obj->data.num = expect_num.value();
        break;
    case ObjEncoding::REDIS_ENCODING_EMBSTR:
        obj->data.ptr = Sds::deserialize_from(ifs);
        break;
    case ObjEncoding::REDIS_ENCODING_RAW:
        obj->data.ptr = Sds::deserialize_from(ifs);
        break;
    default:
        throw std::runtime_error("invalid encoding");
    }
}