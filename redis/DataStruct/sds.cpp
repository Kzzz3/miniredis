module sds;

size_t sds::length() {
	if (type == sds_type::SDS_UNINIT) return 0;

	return access_sdshdr([this](auto psdshdr)->size_t {
		return psdshdr->len;
	});
}

size_t sds::capacity() {
	if (type == sds_type::SDS_UNINIT) return 0;

	return access_sdshdr([this](auto psdshdr)->size_t {
		return psdshdr->alloc;
	});
}

size_t sds::available() {
	return capacity() - length();
}

sds* sds::dilatation(size_t add_len)
{
	if (type == sds_type::SDS_UNINIT)
		return nullptr;

	size_t new_alloc = length() + add_len;
	if (new_alloc <= SDS_MAX_PREALLOC)
		new_alloc *= 2;
	else
		new_alloc += SDS_MAX_PREALLOC;
	return sds::create(buf, length(), new_alloc);
}

sds* sds::copy(sds* str)
{
	if (!str || str->type == sds_type::SDS_UNINIT)
		return nullptr;

	return copy(str->buf, str->length());
}

sds* sds::copy(const char* str, size_t len)
{
	if (type == sds_type::SDS_UNINIT)
		return nullptr;

	sds* ret = nullptr;
	if (len <= capacity()) {
		ret = this;
	}
	else {
		ret = dilatation(len - length());
		destroy(this);
	}

	std::memcpy(ret->buf, str, len);
	ret->access_sdshdr([len](auto psdshdr) {
		psdshdr->len = len;
		psdshdr->buf[psdshdr->len] = '\0';
	});

	return ret;
}

sds* sds::append(sds* str)
{
	if (!str || str->type == sds_type::SDS_UNINIT) 
		return nullptr;

	return append(str->buf, str->length());
}

sds* sds::append(const char* str, size_t len)
{
	if (type == sds_type::SDS_UNINIT)
		return nullptr;

	sds* ret = nullptr;
	if (len <= available()) {
		ret = this;
	}
	else {
		ret = dilatation(len - available());
		destroy(this);
	}

	std::memcpy(ret->buf + ret->length(), str, len);
	ret->access_sdshdr([len](auto psdshdr) {
		psdshdr->len += len;
		psdshdr->buf[psdshdr->len] = '\0';
	});
	return ret;
}

void sds::destroy(sds* s) {
	if (!s || s->type == sds_type::SDS_UNINIT) return;

	s->access_sdshdr([&s](auto psdshdr)->void {
		::operator delete(psdshdr);
		s = nullptr;
	});
}

sds* sds::create(const char* str, size_t len, size_t alloc) {
	alloc = std::max(alloc, len);

	auto allocate = [=]<typename T>() -> sds* {
		sdshdr<T>* hdr = reinterpret_cast<sdshdr<T>*>(::operator new(sizeof(sdshdr<T>) + alloc + 1));

		hdr->len = len;
		hdr->alloc = alloc;
		hdr->type = static_cast<sds_type>(sizeof(T));
		std::memcpy(hdr->buf, str, len);
		hdr->buf[len] = '\0';
		return reinterpret_cast<sds*>(&hdr->type);
	};

	if (alloc < std::numeric_limits<std::uint8_t>::max()) {
		return allocate.operator() < std::uint8_t > ();
	}
	else if (alloc < std::numeric_limits<std::uint16_t>::max()) {
		return allocate.operator() < std::uint16_t > ();
	}
	else if (alloc < std::numeric_limits<std::uint32_t>::max()) {
		return allocate.operator() < std::uint32_t > ();
	}
	else {
		return allocate.operator() < std::uint64_t > ();
	}
}