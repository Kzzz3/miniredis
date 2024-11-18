module sds;

template <typename Func, typename Ret = std::invoke_result_t<Func, sdshdr<std::uint64_t>*>>
auto sds::access_sdshdr(Func&& operation)-> Ret {
	switch (type_) {
	case sds_type::SDS_TYPE_8:
		return operation(reinterpret_cast<sdshdr<std::uint8_t>*>((buf_ - sizeof(sdshdr<std::uint8_t>))));
	case sds_type::SDS_TYPE_16:
		return operation(reinterpret_cast<sdshdr<std::uint16_t>*>((buf_ - sizeof(sdshdr<std::uint16_t>))));
	case sds_type::SDS_TYPE_32:
		return operation(reinterpret_cast<sdshdr<std::uint32_t>*>((buf_ - sizeof(sdshdr<std::uint32_t>))));
	case sds_type::SDS_TYPE_64:
		return operation(reinterpret_cast<sdshdr<std::uint64_t>*>((buf_ - sizeof(sdshdr<std::uint64_t>))));
	default:
		throw std::runtime_error("Invalid SDS type.");
	}
}

size_t sds::length() {
	if (type_ == sds_type::SDS_UNINIT) return 0;
	return access_sdshdr([this](auto psdshdr)->size_t {
		return psdshdr->len;
		});
}

size_t sds::capacity() {
	if (type_ == sds_type::SDS_UNINIT) return 0;
	return access_sdshdr([this](auto psdshdr)->size_t {
		return psdshdr->alloc;
		});
}

size_t sds::available() {
	return capacity() - length();
}

void sds::destroy(sds*& s) {
	if (!s || s->type_ == sds_type::SDS_UNINIT) return;
	s->access_sdshdr([&s](auto psdshdr)->void {
		::operator delete(psdshdr);
		s = nullptr;
		});
}

sds* sds::create(const char* str, size_t len) {
	auto allocate = []<typename T>(const char* str, size_t len) -> sds* {
		sdshdr<T>* hdr = reinterpret_cast<sdshdr<T>*>(::operator new(sizeof(sdshdr<T>) + len + 1));

		hdr->len = len;
		hdr->alloc = len;
		hdr->type = static_cast<sds_type>(sizeof(T));
		std::memcpy(hdr->buf, str, len);
		hdr->buf[len] = '\0';
		return reinterpret_cast<sds*>(&hdr->type);
	};

	if (len < std::numeric_limits<std::uint8_t>::max()) {
		return allocate.operator() < std::uint8_t > (str, len);
	}
	else if (len < std::numeric_limits<std::uint16_t>::max()) {
		return allocate.operator() < std::uint16_t > (str, len);
	}
	else if (len < std::numeric_limits<std::uint32_t>::max()) {
		return allocate.operator() < std::uint32_t > (str, len);
	}
	else {
		return allocate.operator() < std::uint64_t > (str, len);
	}
}