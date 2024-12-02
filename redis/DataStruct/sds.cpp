module sds;

void sds::destroy(sds* s) {
	access_sdshdr(s, [](auto psdshdr)->void {
		std::free(psdshdr);
	});
}

sds* sds::create(const char* str, size_t len, size_t alloc) {
	alloc = std::max(alloc, len);

	auto allocate = [=]<typename T>() -> sds* {
		sdshdr<T>* hdr = reinterpret_cast<sdshdr<T>*>(std::malloc(sizeof(sdshdr<T>) + alloc));

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

size_t sds::length() {
	return access_sdshdr(this, [](auto psdshdr)->size_t {
		return psdshdr->len;
	});
}

size_t sds::capacity() {
	return access_sdshdr(this, [](auto psdshdr)->size_t {
		return psdshdr->alloc;
	});
}

size_t sds::available() {
	return capacity() - length();
}

sds* sds::dilatation(size_t add_len)
{
	size_t len = length();
	size_t new_alloc = capacity() + add_len;

	if (new_alloc <= SDS_MAX_PREALLOC)
		new_alloc *= 2;
	else
		new_alloc += SDS_MAX_PREALLOC;

	auto allocate = [this, len, new_alloc]<typename T>() -> sds* {

		return access_sdshdr(this, [this, len, new_alloc](auto psdshdr) -> sds* {

			sdshdr<T>* new_psdshdr = reinterpret_cast<sdshdr<T>*>(std::realloc(psdshdr, sizeof(sdshdr<T>) + new_alloc));
			size_t offset = sizeof(sdshdr<T>) - sizeof(std::remove_pointer_t<decltype(psdshdr)>);
			std::memmove(new_psdshdr->buf, new_psdshdr->buf - offset, len);

			new_psdshdr->alloc = new_alloc;
			new_psdshdr->len = len;
			new_psdshdr->type = static_cast<sds_type>(sizeof(T));
			new_psdshdr->buf[len] = '\0';

			return reinterpret_cast<sds*>(&new_psdshdr->type);
		});
	};

	if (new_alloc < std::numeric_limits<std::uint8_t>::max()) {
		return allocate.operator() <std::uint8_t> ();
	}
	else if (new_alloc < std::numeric_limits<std::uint16_t>::max()) {
		return allocate.operator() < std::uint16_t > ();
	}
	else if (new_alloc < std::numeric_limits<std::uint32_t>::max()) {
		return allocate.operator() < std::uint32_t > ();
	}
	else {
		return allocate.operator() < std::uint64_t > ();
	}
}

sds* sds::copy(sds* str)
{
	return copy(str->buf, str->length());
}

sds* sds::copy(const char* str, size_t len)
{
	sds* ret = len <= available() ? this : dilatation(len - available());

	std::memcpy(ret->buf, str, len);
	access_sdshdr(ret, [len](auto psdshdr) {
		psdshdr->len = len;
		psdshdr->buf[psdshdr->len] = '\0';
	});
	return ret;
}

sds* sds::append(sds* str)
{
	return append(str->buf, str->length());
}

sds* sds::append(const char* str, size_t len)
{
	sds* ret = len <= available() ? this : dilatation(len - available());

	std::memcpy(ret->buf + ret->length(), str, len);
	access_sdshdr(ret, [len](auto psdshdr) {
		psdshdr->len += len;
		psdshdr->buf[psdshdr->len] = '\0';
	});
	return ret;
}

template <typename Func, typename Ret>
constexpr auto access_sdshdr(sds* str, Func&& operation)-> Ret {
	switch (str->type) {
	case sds_type::SDS_TYPE_8:
		return operation(reinterpret_cast<sdshdr<std::uint8_t>*>(str->buf - sizeof(sdshdr<std::uint8_t>) + 1));
	case sds_type::SDS_TYPE_16:
		return operation(reinterpret_cast<sdshdr<std::uint16_t>*>(str->buf - sizeof(sdshdr<std::uint16_t>) + 1));
	case sds_type::SDS_TYPE_32:
		return operation(reinterpret_cast<sdshdr<std::uint32_t>*>(str->buf - sizeof(sdshdr<std::uint32_t>) + 1));
	case sds_type::SDS_TYPE_64:
		return operation(reinterpret_cast<sdshdr<std::uint64_t>*>(str->buf - sizeof(sdshdr<std::uint64_t>) + 1));
	default:
		throw std::runtime_error("Invalid SDS type.");
	}
}