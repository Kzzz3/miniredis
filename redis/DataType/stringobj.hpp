#pragma once

#include "redisobj.h"
#include "../utility.hpp"
#include "../DataStruct/sds.h"

using std::make_unique;

inline RedisObj* CreateStringObject(Sds* str)
{
	RedisObj* obj = nullptr;
	size_t len = str->length();
	optional<int64_t> value = sds2num<int64_t>(str);

	if (value.has_value()) 
	{
		// integer
		obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
		obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
		obj->data.num = value.value();
	}
	else if (len <= EMBSTR_MAX_LENGTH) 
	{
		// embstr
		obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj) + sizeof(SdsHdr<uint8_t>) + len));
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
		obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
		obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
		obj->data.ptr = Sds::create(str);
	}

	obj->type = ObjType::REDIS_STRING;
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;
}

inline RedisObj* UpdateStringObject(RedisObj* obj, Sds* str)
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
	if (len <= EMBSTR_MAX_LENGTH &&
		obj->encoding == ObjEncoding::REDIS_ENCODING_INT ||
		obj->encoding == ObjEncoding::REDIS_ENCODING_EMBSTR)
	{
		obj = reinterpret_cast<RedisObj*>(std::realloc(obj, sizeof(RedisObj) + sizeof(SdsHdr<uint8_t>) + len));
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
	if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT||
		obj->encoding == ObjEncoding::REDIS_ENCODING_EMBSTR)
	{
		obj = reinterpret_cast<RedisObj*>(std::realloc(obj, sizeof(RedisObj)));
		obj->data.ptr = Sds::create(str);
	}
	else
	{
		obj->data.ptr = reinterpret_cast<Sds*>(obj->data.ptr)->copy(str);
	}
	obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
	obj->lru = GetSecTimestamp();

	return obj;
}

inline auto StringObjGet(RedisObj* obj)
{
	Sds* value = nullptr;
	std::vector<Sds*> result;
	if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
	{
		return make_unique<ValueRef>(num2sds(obj->data.num), nullptr);
	}
	return make_unique<ValueRef>(reinterpret_cast<Sds*>(obj->data.ptr), obj);
}