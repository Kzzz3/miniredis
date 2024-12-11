#include "ziplist.h"

ZipList* ZipList::create()
{
	ZipList* zl = reinterpret_cast<ZipList*>(std::malloc(sizeof(ZipList)));
	zl->total_bytes = 0;
	zl->last_offset = 0;
	zl->items_num = 0;
	zl->buf[0] = ZIPLIST_END;
	return zl;
}

void ZipList::destroy(ZipList* zl)
{
	std::free(zl);
}

ZipList* ZipList::pop_back()
{
	if (items_num == 0) return this;

	ZlEntry entry;
	entryDecode(buf + last_offset, entry);

	ZipList* new_zl = reinterpret_cast<ZipList*>(std::realloc(this,sizeof(ZipList) + last_offset));
	new_zl->total_bytes = new_zl->last_offset;
	new_zl->last_offset -= entry.prevrawlen;
	new_zl->items_num--;
	new_zl->buf[new_zl->total_bytes] = ZIPLIST_END;

	return new_zl;
}

ZipList* ZipList::pop_front()
{
	if (items_num == 0) return this;

	ZlEntry entry;
	entryDecode(buf, entry);
	size_t entry_size = entry.prevrawlensize + entry.lensize + entry.len;

	memmove(buf, buf + entry_size, total_bytes - entry_size);
	ZipList* new_zl = reinterpret_cast<ZipList*>(std::realloc(this, sizeof(ZipList) + total_bytes - entry_size));
	new_zl->total_bytes -= entry_size;
	new_zl->last_offset = new_zl->items_num == 1 ? 0 : new_zl->last_offset - entry_size;
	new_zl->items_num--;
	new_zl->buf[new_zl->total_bytes] = ZIPLIST_END;

	return adjustSubsequentNodes(new_zl, 0, 0 - entry_size);
}

uint8_t* ZipList::index(int index)
{
	index = (index % items_num + items_num) % items_num;

	uint8_t* p = buf;
	for (int i = 0; i < index; ++i)
	{
		ZlEntry entry;
		entryDecode(p, entry);
		p += entry.prevrawlensize + entry.lensize + entry.len;
	}
	return p;
}

uint8_t* ZipList::next(uint8_t* p)
{
	if (p[0] == ZIPLIST_END) return nullptr;

	ZlEntry entry;
	entryDecode(p, entry);
	return p + entry.prevrawlensize + entry.lensize + entry.len;
}

uint8_t* ZipList::prev(uint8_t* p)
{
	if (p == buf) return nullptr;

	ZlEntry entry;
	entryDecode(p, entry);
	return p - entry.prevrawlen;
}

ZipList* ZipList::push_back(uint8_t* str, size_t len)
{
	return insert(buf + total_bytes, str, len);
}

ZipList* ZipList::push_front(uint8_t* str, size_t len)
{
	return insert(buf, str, len);
}

ZipList* ZipList::insert(uint8_t* p, uint8_t* str, size_t len)
{
	ZlEntry new_entry;
	if (items_num == 0)
	{
		new_entry.prevrawlen = 0;
		new_entry.prevrawlensize = 1;
	}
	else if (p[0] != ZIPLIST_END)
	{
		ZlEntry entry;
		entryDecode(p, entry);
		new_entry.prevrawlen = entry.prevrawlen;
		new_entry.prevrawlensize = entry.prevrawlensize;
	}
	else
	{
		ZlEntry entry;
		entryDecode(buf + last_offset, entry);
		new_entry.prevrawlen = entry.prevrawlensize + entry.lensize + entry.len;
		new_entry.prevrawlensize = new_entry.prevrawlen < 254 ? 1 : 5;
	}

	if (!tryIntEncode(str, len, new_entry))
	{
		strEncode(str, len, new_entry);
	}

	size_t offset = p - buf;
	size_t new_entry_size = new_entry.prevrawlensize + new_entry.lensize + new_entry.len;
	ZipList* new_zl = reinterpret_cast<ZipList*>(std::realloc(this, sizeof(ZipList) + total_bytes + new_entry_size));

	memmove(new_zl->buf + offset + new_entry_size, new_zl->buf + offset, new_zl->total_bytes - offset);
	entryEncode(new_zl->buf + offset, new_entry);

	new_zl->total_bytes += new_entry_size;
	new_zl->last_offset = new_zl->items_num == 0 ? 0 : offset == new_zl->total_bytes - new_entry_size ? offset : last_offset + new_entry_size;
	new_zl->items_num++;
	new_zl->buf[new_zl->total_bytes] = ZIPLIST_END;

	return adjustSubsequentNodes(new_zl, offset + new_entry_size, new_entry_size - new_entry.prevrawlen);
}

void entryEncode(uint8_t* p, ZlEntry& entry)
{
	//prevrawlen
	if (entry.prevrawlensize == 1)
	{
		p[0] = entry.prevrawlen;
	}
	else
	{
		p[0] = ZIPLIST_PREVLEN_FIRST;
		memcpy(p + 1, &entry.prevrawlen, 4);
	}
	p += entry.prevrawlensize;

	//encoding
	if (isStr(entry.encoding))
	{
		if (entry.lensize == 1)
		{
			p[0] = (entry.len & 0x3f) | entry.encoding;
		}
		else if (entry.lensize == 2)
		{
			uint16_t temp = 0x3fff & entry.len;
			p[0] = temp >> 8 | entry.encoding;
			p[1] = temp & 0xff;
		}
		else
		{
			p[0] = entry.encoding;
			memcpy(p + 1, &entry.len, 4);
		}
		p += entry.lensize;
		memcpy(p, entry.data.ptr, entry.len);

		return;
	}

	p[0] = entry.encoding;
	p += entry.lensize;
	reinterpret_cast<int64_t*>(p)[0] = entry.data.num;
}

void entryDecode(uint8_t* p, ZlEntry& entry)
{
	//prevrawlen
	if (p[0] == ZIPLIST_PREVLEN_FIRST)
	{
		memcpy(&entry.prevrawlen, p + 1, 4);
		entry.prevrawlensize = 5;
	}
	else
	{
		entry.prevrawlen = p[0];
		entry.prevrawlensize = 1;
	}
	p += entry.prevrawlensize;

	//encoding
	entry.encoding = p[0] & ZIPLIST_ENCODING_HEADER;
	if (isStr(entry.encoding))
	{
		if (entry.encoding == ZIPLIST_STR_32B)
		{
			memcpy(&entry.len, p + 1, 4);
			entry.lensize = 5;
		}
		else if (entry.encoding == ZIPLIST_STR_14B)
		{
			entry.len = (p[0] & 0x3f) << 8 | p[1];
			entry.lensize = 2;
		}
		else
		{
			entry.len = p[0] & 0x3f;
			entry.lensize = 1;
		}
		entry.data.ptr = p + entry.lensize;
	}
	else
	{
		entry.len = 8;
		entry.lensize = 1;
		entry.data.num = reinterpret_cast<int64_t*>(p + 1)[0];
	}
}

bool tryIntEncode(uint8_t* str, size_t len, ZlEntry& entry)
{
	std::string temp(reinterpret_cast<char*>(str), len);
	try
	{
		auto value = str2num<int64_t>(reinterpret_cast<char*>(str), len);
		if (!value.has_value()) return false; // Ensure full match

		entry.len = 8;
		entry.lensize = 1;
		entry.encoding = ZIPLIST_INT_64B; // Update encoding flag
		entry.data.num = value.value();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void strEncode(uint8_t* str, size_t len, ZlEntry& entry)
{
	entry.data.ptr = str;
	entry.len = len;
	if (len <= 0x3f)
	{
		entry.lensize = 1;
		entry.encoding = ZIPLIST_STR_06B;
	}
	else if (len <= 0x3fff)
	{
		entry.lensize = 2;
		entry.encoding = ZIPLIST_STR_14B;
	}
	else
	{
		entry.lensize = 5;
		entry.encoding = ZIPLIST_STR_32B;
	}
}

ZipList* adjustSubsequentNodes(ZipList* zl, size_t offset, int diff)
{
	ZipList* ret = zl;
	while (ret->buf[offset] != ZIPLIST_END)
	{
		ZlEntry entry;
		entryDecode(ret->buf + offset, entry);

		size_t oldprevlensize = entry.prevrawlensize;

		entry.prevrawlen += diff;
		entry.prevrawlensize = entry.prevrawlen < 254 ? 1 : 5;

		if (oldprevlensize == entry.prevrawlensize)
		{
			entryEncode(ret->buf + offset, entry);
			break;
		}

		diff = entry.prevrawlensize - oldprevlensize;
		size_t old_entry_size = oldprevlensize + entry.lensize + entry.len;
		size_t new_entry_size = entry.prevrawlensize + entry.lensize + entry.len;

		ZipList* new_zl = nullptr;
		if (diff<0)
		{
			memmove(ret->buf + offset + new_entry_size, ret->buf + offset + old_entry_size, ret->total_bytes - offset - old_entry_size);
			new_zl = reinterpret_cast<ZipList*>(std::realloc(ret, sizeof(ZipList) + ret->total_bytes + diff));
		}
		else
		{
			new_zl = reinterpret_cast<ZipList*>(std::realloc(ret, sizeof(ZipList) + ret->total_bytes + diff));
			memmove(new_zl->buf + offset + new_entry_size, new_zl->buf + offset + old_entry_size, ret->total_bytes - offset - old_entry_size);
		}
		entryEncode(new_zl->buf + offset, entry);

		new_zl->total_bytes += (entry.prevrawlensize - oldprevlensize);
		new_zl->last_offset += (entry.prevrawlensize - oldprevlensize);
		new_zl->items_num = new_zl->items_num;
		new_zl->buf[new_zl->total_bytes] = ZIPLIST_END;

		offset += new_entry_size;
		ret = new_zl;
	}
	return ret;
}