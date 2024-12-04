#pragma once

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/sds.h"

inline RedisObj* CreateStringObject(const char* str, size_t len)
{
	// integer
	auto value = str2num<int64_t>(str, len);
	if (value.has_value()) {
		RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
		obj->type = ObjType::REDIS_STRING;
		obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
		obj->data.num = value.value();
		obj->lru = GetSecTimestamp();
		obj->refcount = 1;
		return obj;
	}

	// embstr
	if (len <= EMBSTR_MAX_LENGTH) {
		RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj) + sizeof(SdsHdr<uint8_t>) + len));
		obj->type = ObjType::REDIS_STRING;
		obj->encoding = ObjEncoding::REDIS_ENCODING_EMBSTR;
		obj->data.ptr = reinterpret_cast<std::byte*>(obj) + sizeof(RedisObj) + sizeof(uint8_t) * 2;
		obj->lru = GetSecTimestamp();
		obj->refcount = 1;

		SdsHdr<uint8_t>* hdr = reinterpret_cast<SdsHdr<uint8_t>*>(reinterpret_cast<std::byte*>(obj) + sizeof(RedisObj));
		hdr->len = len;
		hdr->alloc = len;
		hdr->type = SdsType::SDS_TYPE_8;
		std::memcpy(hdr->buf, str, len);
		hdr->buf[len] = '\0';
		return obj;
	}

	// raw
	RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_STRING;
	obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
	obj->data.ptr = Sds::create(str, len, len);
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}