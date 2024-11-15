module sds;

template <typename Func>
void sds::access_hdr(Func&& operation) {
	if (!buf_) {
		using ReturnType = decltype(std::declval<Func>().template operator() < std::uint8_t > ());
		return ReturnType{};
	}

	sds_type type = static_cast<sds_type>(buf_[-1]);

	switch (type) {
	case sds_type::SDS_TYPE_8:
		operation.template operator() < std::uint8_t > (); break;
	case sds_type::SDS_TYPE_16:
		operation.template operator() < std::uint16_t > (); break;
	case sds_type::SDS_TYPE_32:
		operation.template operator() < std::uint32_t > (); break;
	case sds_type::SDS_TYPE_64:
		operation.template operator() < std::uint64_t > (); break;
	default:
		throw std::runtime_error("Unknown sds type");
	}
}

size_t sds::length() {
	if (!buf_) return 0;

	size_t len = 0;
	access_hdr([this, &len]<typename T>() {
		len = reinterpret_cast<sdshdr<T>*>(buf_ - sizeof(sdshdr<T>))->len;
	});
	return len;
}

size_t sds::capacity() {
	if (!buf_) return 0;

	size_t capacity = 0;
	access_hdr([this, &capacity]<typename T>() {
		capacity = reinterpret_cast<sdshdr<T>*>(buf_ - sizeof(sdshdr<T>))->alloc;
	});
	return capacity;
}

size_t sds::available() {
	return capacity() - length();
}

void sds::free() {
	if (!buf_) return;

	access_hdr([this]<typename T>() {
		auto* hdr = reinterpret_cast<sdshdr<T>*>(buf_ - sizeof(sdshdr<T>));
		hdr->~sdshdr<T>();
		::operator delete(hdr);
		buf_ = nullptr;
	});
}

void sds::init(const char* str, size_t len) {
	free();

	auto allocate_sds = [this]<typename T>(const char* str, size_t len) {
		sdshdr<T>* hdr = static_cast<sdshdr<T>*>(::operator new(sizeof(sdshdr<T>) + len + 1));

		hdr->len = len;
		hdr->alloc = len;
		hdr->type = static_cast<sds_type>(sizeof(T) - 1);
		std::memcpy(hdr->buf, str, len);
		hdr->buf[len] = '\0';
		buf_ = hdr->buf;
	};

	if (len < std::numeric_limits<std::uint8_t>::max()) {
		allocate_sds.operator() < std::uint8_t > (str, len);
	}
	else if (len < std::numeric_limits<std::uint16_t>::max()) {
		allocate_sds.operator() < std::uint16_t > (str, len);
	}
	else if (len < std::numeric_limits<std::uint32_t>::max()) {
		allocate_sds.operator() < std::uint32_t > (str, len);
	}
	else {
		allocate_sds.operator() < std::uint64_t > (str, len);
	}
}