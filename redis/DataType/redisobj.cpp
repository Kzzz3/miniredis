module redisobj;

RedisObj* CreateStringObject(const char* str, size_t len)
{
	// integer
	auto value = str2ll(str, len);
	if (value.has_value()) {
		RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
		obj->type = ObjType::REDIS_STRING;
		obj->encoding = ObjEncoding::REDIS_ENCODING_INT;
		obj->ptr = reinterpret_cast<void*>(value.value());
		obj->lru = GetSecTimestamp();
		obj->refcount = 1;
		return obj;
	}

	// embstr
	if (len <= EMBSTR_MAX_LENGTH) {
		RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj) + sizeof(sdshdr<uint8_t>) + len));
		obj->type = ObjType::REDIS_STRING;
		obj->encoding = ObjEncoding::REDIS_ENCODING_EMBSTR;
		obj->ptr = reinterpret_cast<std::byte*>(obj) + sizeof(RedisObj) + sizeof(uint8_t) * 2;
		obj->lru = GetSecTimestamp();
		obj->refcount = 1;
		return obj;
	}

	// raw
	RedisObj* obj = reinterpret_cast<RedisObj*>(std::malloc(sizeof(RedisObj)));
	obj->type = ObjType::REDIS_STRING;
	obj->encoding = ObjEncoding::REDIS_ENCODING_RAW;
	obj->ptr = sds::create(str, len, len);
	obj->lru = GetSecTimestamp();
	obj->refcount = 1;
	return obj;

}

RedisObj* CreateHashObject(const char* field, const char* value, size_t field_len, size_t value_len)
{
	return nullptr;
}

RedisObj* CreateListObject(const char* value, size_t len)
{
	return nullptr;
}

RedisObj* CreateSetObject(const char* member, size_t len)
{
	return nullptr;
}

RedisObj* CreateZsetObject(const char* member, double score)
{
	return nullptr;
}