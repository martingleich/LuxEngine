#include "core/lxString.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxArray.h"

namespace lux
{

const string string::EMPTY = string();

string::string() :
	m_Data(nullptr),
	m_Size(0),
	m_Allocated(0),
	m_Length(0)
{
}

string::string(const char* data, size_t length) :
	m_Data(nullptr),
	m_Size(0),
	m_Allocated(0),
	m_Length(0)
{
	if(!data || !length) {
		Reserve(1);
		Data()[0] = 0;
		return;
	}

	size_t size;
	if(length == std::numeric_limits<size_t>::max()) {
		length = core::StringLengthUTF8(data);
		size = strlen(data);
	} else {
		const char* cur = data;
		size_t count = 0;
		while(count < length && core::AdvanceCursorUTF8(cur))
			++count;
		size = cur - data;
		if(count != length)
			length = count;
	}

	Reserve(size);
	memcpy(Data(), data, size);
	Data()[size] = 0;

	m_Length = length;
	m_Size = size;
}

string::string(const string& other) :
	m_Data(nullptr),
	m_Size(0),
	m_Allocated(0),
	m_Length(0)
{
	Reserve(other.m_Size);
	memcpy(Data(), other.Data_c(), other.m_Size + 1);

	m_Length = other.m_Length;
	m_Size = other.m_Size;
}

string::string(string&& old) :
	m_Data(old.m_Data),
	m_Size(old.m_Size),
	m_Allocated(old.m_Allocated),
	m_Length(old.m_Length)
{
	old.m_Data = nullptr;
	old.m_Size = 0;
	old.m_Allocated = 0;
	old.m_Length = 0;
}

string::~string()
{
	if(!IsShortString())
		delete[] m_Data;
}


string string::Copy()
{
	return string(*this);
}

void string::Reserve(size_t size)
{
	size_t alloc = GetAllocated();
	size_t newAlloc = size + 1;
	if(newAlloc <= alloc)
		return;

	char* newData;
	bool isShort = (newAlloc <= MaxShortStringBytes());
	if(isShort)
		newData = (char*)&m_Data;
	else
		newData = new char[newAlloc];

	memcpy(newData, Data_c(), m_Size + 1);

	if(!IsShortString())
		delete[] m_Data;
	if(!isShort)
		m_Data = newData;

	SetAllocated(newAlloc, isShort);
}

string& string::operator=(const string& other)
{
	if(this == &other)
		return *this;

	Reserve(other.m_Size);
	memcpy(Data(), other.Data_c(), other.m_Size + 1);
	m_Size = other.m_Size;
	m_Length = other.m_Length;

	return *this;
}

string& string::operator=(const char* other)
{
	if(Data_c() == other)
		return *this;
	if(!other) {
		Clear();
		return *this;
	}

	const size_t size = strlen(other);
	Reserve(size);
	memcpy(Data(), other, size + 1);
	m_Size = size;
	m_Length = core::StringLengthUTF8(other);

	return *this;
}

string& string::operator=(string&& old)
{
	this->~string();
	m_Data = old.m_Data;
	m_Size = old.m_Size;
	m_Allocated = old.m_Allocated;
	m_Length = old.m_Length;
	old.m_Data = nullptr;
	old.m_Allocated = 0;
	old.m_Length = 0;
	old.m_Size = 0;

	return *this;
}

string& string::operator+=(const string_type& str)
{
	return Append(str);
}
string string::operator+(const string_type& str) const
{
	string n(*this);
	n += str;
	return n;
}

bool string::operator==(const string_type& other) const
{
	other.EnsureSize();
	if(m_Size != other.size)
		return false;

	return (memcmp(Data(), other.data, m_Size) == 0);
}

bool string::operator<(const string_type& other) const
{
	other.EnsureSize();

	size_t s = m_Size < other.size ? m_Size : other.size;

	// TODO: This is not the correct way to compare strings.
	return (memcmp(Data(), other.data, s) < 0);
}

bool string::operator!=(const string_type& other) const
{
	return !(*this == other);
}

bool string::Equal(const string_type& other) const
{
	return (*this == other);
}

bool string::EqualCaseInsensitive(const string_type& other) const
{
	other.EnsureSize();
	if(m_Size != other.size)
		return false;

	if(m_Size == 0)
		return true;

	const char* a = Data();
	const char* b = other.data;
	u32 ac;
	u32 bc;
	do {
		ac = core::AdvanceCursorUTF8(a);
		bc = core::AdvanceCursorUTF8(b);

		if(!core::IsEqualCaseInsensitive(ac, bc))
			return false;
	} while(ac && bc);

	return (ac == bc);
}

bool string::SmallerCaseInsensitive(const string_type& other) const
{
	const char* a = Data();
	const char* b = other.data;
	u32 ac;
	u32 bc;
	do {
		ac = core::AdvanceCursorUTF8(a);
		bc = core::AdvanceCursorUTF8(b);

		u32 iac = core::ToLowerChar(ac);
		u32 ibc = core::ToLowerChar(bc);
		if(iac < ibc)
			return true;
		if(iac > ibc)
			return false;
	} while(ac && bc);

	return (ac < bc);
}

string::ConstIterator string::Insert(ConstIterator pos, const string_type& other, size_t count)
{
	lxAssert(pos.m_First == Data());

	other.EnsureSize();
	size_t size;
	if(count == std::numeric_limits<size_t>::max()) {
		count = core::StringLengthUTF8(other.data);
		size = other.size;
	} else {
		const char* cur = other.data;
		size_t c = 0;
		while(c < count && core::AdvanceCursorUTF8(cur))
			++c;
		size = cur - other.data;
	}
	if(size == 0)
		return pos;
	size_t newSize = m_Size + size;
	size_t pos_off = pos.Pointer() - Data();
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	memmove(data + pos_off + size, data + pos_off, m_Size - pos_off + 1);

	// Place the insertion string.
	memcpy(data + pos_off, other.data, size);

	m_Size += size;
	m_Length += count;

	return ConstIterator(data + pos_off + size, Data_c());
}

string::ConstIterator string::Insert(ConstIterator pos, ConstIterator first, ConstIterator end)
{
	// This assumes, string::ConstIterator point to a continues block of memory until end.
	// This is always the case for string::ConstIterator.

	size_t size = end.Pointer() - first.Pointer();
	size_t count = core::IteratorDistance(first, end);

	size_t newSize = m_Size + size;
	size_t pos_off = Data() - pos.Pointer();
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	memmove(data + pos_off + size, data + pos_off, m_Size - pos_off + 1);

	// Place the insertion string.
	memcpy(data + pos_off, first.Pointer(), size);

	m_Size += size;
	m_Length += count;

	return ConstIterator(data + pos_off + size, Data_c());
}

string& string::AppendRaw(const char* data, size_t bytes)
{
	Reserve(bytes);
	memcpy(Data() + m_Size, data, bytes);
	m_Length += core::StringLengthUTF8(data);
	m_Size += bytes;
	Data()[m_Size] = 0;
	return *this;
}

string& string::Append(const string_type& other, size_t count)
{
	Insert(End(), other, count);
	return *this;
}

string& string::Append(ConstIterator first, ConstIterator end)
{
	Insert(End(), first, end);
	return *this;
}

string& string::Append(ConstIterator character)
{
	PushCharacter(character.Pointer());
	return *this;
}

string& string::Append(u32 character)
{
	u8 buffer[6];
	u8* end = core::CodePointToUTF8(character, buffer);
	const size_t size = end - buffer;
	Reserve(m_Size + size);

	memcpy(Data() + m_Size, buffer, size);
	m_Size += size;
	Data()[m_Size] = 0;

	++m_Length;

	return *this;
}

void string::Resize(size_t newLength, const string_type& filler)
{
	if(newLength == 0) {
		m_Length = 0;
		m_Size = 0;
	} else if(newLength <= m_Length) {
		const char* data = Data();
		const char* ptr = data + m_Size;
		while(m_Length > newLength) {
			core::RetractCursorUTF8(ptr);
			--m_Length;
		}

		m_Size -= (data + m_Size) - ptr;
	} else {
		filler.EnsureSize();
		if(filler.size == 0)
			throw core::InvalidArgumentException("filler", "length(filler) > 0");

		size_t fillerLength = filler.size == 1 ? 1 : core::StringLengthUTF8(filler.data);
		size_t addLength = newLength - m_Length;

		size_t elemCount = addLength / fillerLength;
		size_t addBytes = elemCount*filler.size;
		size_t remCount = 0;
		size_t neededBytes = elemCount * filler.size;
		if(addLength%fillerLength != 0) {
			neededBytes += filler.size;
			remCount = addLength%fillerLength;
		}

		Reserve(m_Size + neededBytes);
		if(filler.size != 1) {
			char* cur = Data() + m_Size;
			// Copy whole filler strings.
			for(size_t i = 0; i < elemCount; ++i) {
				memcpy(cur, filler.data, filler.size);
				cur += filler.size;
			}
			// Copy the last partial string.
			const char* fillCur = filler.data;
			size_t i = 0;
			while(i < remCount) {
				*cur++ = *fillCur;
				if((*fillCur & 0xC0) != 0x80)
					++i;
				++addBytes;
			}
		} else {
			char* cur = Data() + m_Size;
			memset(cur, *filler.data, elemCount);
		}

		m_Size += addBytes;
		m_Length += addLength;
	}

	Data()[m_Size] = 0;
}

void string::Clear()
{
	Data()[0] = 0;
	m_Length = 0;
	m_Size = 0;
}

size_t string::Size() const
{
	return m_Size;
}

size_t string::Length() const
{
	return m_Length;
}

bool string::IsEmpty() const
{
	return (Size() == 0);
}

const char* string::Data_c() const
{
	return Data();
}

const char* string::Data() const
{
	if(IsShortString() || !m_Data)
		return (const char*)&m_Data;
	return m_Data;
}

char* string::Data()
{
	if(IsShortString() || !m_Data)
		return (char*)&m_Data;
	return m_Data;
}

void string::PushByte(u8 byte)
{
	if(m_Size + 1 >= GetAllocated())
		Reserve(GetAllocated() * 2 + 1);

	if(byte == 0)
		throw core::InvalidArgumentException("byte", "byte must not be null");

	Data()[m_Size] = byte;
	Data()[m_Size + 1] = 0;

	++m_Size;
	if((byte & 0xC0) != 0x80) // Bytes starting with 10 are continuation bytes.
		++m_Length;
}

string::ConstIterator string::Begin() const
{
	return ConstIterator(Data_c() - 1, Data_c());
}

string::ConstIterator string::First() const
{
	return ConstIterator(Data_c(), Data_c());
}

string::ConstIterator string::Last() const
{
	if(m_Size > 0)
		return End() - 1;
	else
		return End();
}

string::ConstIterator string::End() const
{
	return ConstIterator(Data_c() + m_Size, Data_c());
}

bool string::StartsWith(const string_type& data, ConstIterator first) const
{
	if(first == ConstIterator::Invalid())
		first = First();

	lxAssert(first.m_First == Data());

	if(data.data[0] == 0)
		return true;

	const char* strCur = first.Pointer();
	for(auto it = data.data; *it != 0; ++it, ++strCur) {
		if(*it != *strCur)
			return false;
	}

	return true;
}

bool string::EndsWith(const string_type& data, ConstIterator end) const
{
	if(end == ConstIterator::Invalid())
		end = End();

	lxAssert(end.m_First == Data());

	if(data.data[0] == 0)
		return true;

	data.EnsureSize();
	const char* dataCur = data.data + data.size - 1;
	const char* strCur = end.Pointer() - 1;
	size_t i = 0;
	for(auto it = dataCur; i < data.size; --it, --strCur, ++i) {
		if(*it != *strCur)
			break;
	}

	return (i == data.size);
}

size_t string::Replace(const string_type& replace, const string_type& search, ConstIterator first, ConstIterator end)
{
	if(end == ConstIterator::Invalid())
		end = End();

	if(first == ConstIterator::Invalid())
		first = First();

	size_t count = 0;
	size_t length = core::StringLengthUTF8(search.data);
	ConstIterator it = Find(search, first, end);
	while(it != end) {
		size_t endOffset = end.Pointer() - Data_c();
		it = ReplaceRange(replace, it, length);
		end = ConstIterator(Data_c() + endOffset, Data_c());
		it = Find(search, it, end);

		++count;
	}

	return count;
}

string::ConstIterator string::ReplaceRange(const string_type& replace, ConstIterator rangeFirst, ConstIterator rangeEnd)
{
	if(rangeEnd == ConstIterator::Invalid())
		rangeEnd = End();

	return ReplaceRange(replace, rangeFirst, core::IteratorDistance(rangeFirst, rangeEnd));
}

string::ConstIterator string::ReplaceRange(const string_type& replace, ConstIterator rangeFirst, size_t count)
{
	lxAssert(rangeFirst.m_First == Data());

	replace.EnsureSize();
	size_t replacedSize = (rangeFirst + count).Pointer() - rangeFirst.Pointer();

	if(m_Size + replace.size < replacedSize)
		throw core::InvalidArgumentException("count", "count must not be to large.");

	size_t newSize = m_Size - replacedSize + replace.size;
	size_t replaceOffset = rangeFirst.Pointer() - Data_c();
	Reserve(newSize);

	char* data = Data();
	size_t restSize = m_Size - replaceOffset - replacedSize;
	memmove(data + replaceOffset + replace.size, data + replaceOffset + replacedSize, restSize + 1); // Including NUL
	memcpy(data + replaceOffset, replace.data, replace.size);

	m_Size = newSize;
	m_Length = m_Length - count + core::StringLengthUTF8(replace.data);

	return ConstIterator(data + replaceOffset + replacedSize, data);
}

string::ConstIterator string::Find(const string_type& search, ConstIterator first, ConstIterator end) const
{
	if(first == ConstIterator::Invalid())
		first = First();
	if(end == ConstIterator::Invalid())
		end = End();

	lxAssert(first.m_First == Data());
	lxAssert(end.m_First == Data());

	search.EnsureSize();
	if(search.size == 0)
		return end;

	const char* searchFirst = search.data;
	while(first.Pointer() + search.size <= end.Pointer()) {
		if(memcmp(searchFirst, first.Pointer(), search.size) == 0)
			return first;
		++first;
	}

	return end;
}

string::ConstIterator string::FindReverse(const string_type& search, ConstIterator first, ConstIterator end) const
{
	if(first == ConstIterator::Invalid())
		first = First();
	if(end == ConstIterator::Invalid())
		end = End();

	lxAssert(first.m_First == Data());
	lxAssert(end.m_First == Data());

	if(first == end)
		return end;

	search.EnsureSize();
	if(search.size == 0)
		return end;

	ConstIterator cur = end;
	const char* searchFirst = search.data;

	while(cur.Pointer() - search.size >= first.Pointer()) {
		if(memcmp(cur.Pointer() - search.size, searchFirst, search.size) == 0)
			return ConstIterator(cur.Pointer() - search.size, Data_c());
		--cur;
	}

	return end;
}

string string::SubString(ConstIterator first, size_t count) const
{
	lxAssert(first.m_First == Data());

	return string(first.Pointer(), count);
}

string string::SubString(ConstIterator first, ConstIterator end) const
{
	lxAssert(first.m_First == Data());
	lxAssert(end.m_First == Data());

	string out;
	ConstIterator it = first;
	while(it != end) {
		out.Append(it);
		++it;
	}

	return out;
}

size_t string::Pop(size_t count)
{
	ConstIterator it = Last();
	size_t oldCount = count;
	while(it != First() && count > 0) {
		--it;
		--count;
	}

	const char* last = (it + 1).Pointer();
	size_t newSize = last - Data();
	if(it == First() && count > 0) {
		newSize = 0;
		--count;
	}

	Data()[newSize] = 0;
	m_Size = newSize;
	size_t removed = oldCount - count;
	m_Length -= removed;

	return removed;
}

string::ConstIterator string::Remove(ConstIterator pos, size_t count)
{
	lxAssert(pos.m_First == Data());

	size_t removeSize = (pos + count).Pointer() - pos.Pointer();

	if(removeSize > m_Size)
		throw core::InvalidArgumentException("count", "count must not be to large.");

	size_t newSize = m_Size - removeSize;

	char* data = Data();
	size_t removeOffset = pos.Pointer() - data;
	size_t restSize = m_Size - removeOffset - removeSize;
	memmove(data + removeOffset, data + removeOffset + removeSize, restSize + 1); // Including NUL

	m_Size = newSize;
	m_Length = m_Length - count;

	return ConstIterator(data + removeOffset, m_Data);
}

string::ConstIterator string::Remove(ConstIterator from, ConstIterator to)
{
	return Remove(from, core::IteratorDistance(from, to));
}

string& string::RStrip(ConstIterator end)
{
	if(end == ConstIterator::Invalid())
		end = End();

	if(end == First())
		return *this;

	lxAssert(end.m_First == Data());

	char* data = Data();

	ConstIterator last = end - 1;
	size_t lastSize = strlen(last.Pointer());
	ConstIterator begin = Begin();
	size_t removed = 0;
	while(last != begin && core::IsSpace(*last)) {
		--last;
		++removed;
	}

	if(last == begin)
		m_Size = 0;
	else
		m_Size = last.Pointer() - data + lastSize;

	m_Length -= removed;
	data[m_Size] = 0;

	return *this;
}

string& string::LStrip(ConstIterator first)
{
	if(first == ConstIterator::Invalid())
		first = First();

	lxAssert(first.m_First == Data());

	size_t offset = first.Pointer() - Data_c();
	auto end = End();
	size_t count = 0;
	while(first != end && core::IsSpace(*first)) {
		++first;
		++count;
	}

	char* base = Data() + offset;
	size_t removed = first.Pointer() - base;
	memmove(base, first.Pointer(), m_Size - offset - removed);
	m_Size -= removed;
	m_Length -= count;

	return *this;
}

size_t string::Split(u32 ch, string* outArray, size_t maxCount) const
{
	if(maxCount == 0)
		return 0;

	string* cur = outArray;
	size_t count = 1;
	cur->Clear();
	for(auto it = First(); it != End(); ++it) {
		if(*it == ch) {
			if(count == maxCount)
				return maxCount;
			++cur;
			++count;
			cur->Clear();
		} else {
			cur->Append(*it);
		}
	}

	return count;
}

core::array<string> string::Split(u32 ch) const
{
	core::array<string> out;
	string buffer;
	for(auto it = First(); it != End(); ++it) {
		if(*it == ch) {
			out.PushBack(std::move(buffer));
			buffer.Clear();
		} else {
			buffer.Append(*it);
		}
	}

	return out;
}

EStringType string::Classify() const
{
	size_t alphaCount = 0;
	size_t digitCount = 0;
	size_t spaceCount = 0;
	size_t upperCount = 0;
	size_t lowerCount = 0;
	for(auto it = First(); it != End(); ++it) {
		uint32_t c = *it;
		if(core::IsLower(c))
			++lowerCount;
		else if(core::IsUpper(c))
			++upperCount;

		if(core::IsAlpha(c))
			++alphaCount;
		else if(core::IsDigit(c))
			++digitCount;
		else if(core::IsSpace(c))
			++spaceCount;
	}

	EStringType out = (EStringType)0;
	if(m_Length == 0)
		out |= EStringType::Empty;

	if(lowerCount == alphaCount && alphaCount > 0)
		out |= EStringType::Lower;
	else if(upperCount == alphaCount && alphaCount > 0)
		out |= EStringType::Upper;

	if(alphaCount == m_Length && alphaCount > 0)
		out |= EStringType::Alpha;
	else if(digitCount == m_Length && digitCount > 0)
		out |= EStringType::Digit;
	else if(alphaCount + digitCount == m_Length && alphaCount > 0 && digitCount > 0)
		out |= EStringType::AlphaNum;
	else if(spaceCount == m_Length && spaceCount > 0)
		out |= EStringType::Space;

	return out;
}

string string::GetLower() const
{
	string out;
	out.Reserve(Size());

	for(auto it = First(); it != End(); ++it) {
		u32 c = core::ToLowerChar(*it);
		out.Append(c);
	}

	return out;
}

string string::GetUpper() const
{
	string out;
	out.Reserve(Size());

	for(auto it = First(); it != End(); ++it) {
		u32 c = core::ToUpperChar(*it);
		out.Append(c);
	}

	return out;
}

bool string::IsShortString() const
{
	// Check the most siginficant bit.
	return ((m_Allocated&((size_t)1 << (sizeof(m_Allocated) * 8 - 1))) != 0) || m_Allocated == 0;
}

size_t string::GetAllocated() const
{
	static const size_t flag = ((size_t)1 << (sizeof(m_Allocated) * 8 - 1));
	return (m_Allocated & (flag - 1));
}

void string::SetAllocated(size_t a, bool short_string)
{
	static const size_t flag = ((size_t)1 << (sizeof(m_Allocated) * 8 - 1));
	m_Allocated = a | (short_string ? flag : 0);
}

size_t string::MaxShortStringBytes() const
{
	return sizeof(m_Data);
}

void string::PushCharacter(const char* ptr)
{
	if(m_Size + 6 >= GetAllocated()) // Maximal 6 utf-8 bytes
		Reserve(GetAllocated() * 2 + 6);

	if(*ptr == 0)
		throw core::InvalidArgumentException("byte", "byte must not be null");

	Data()[m_Size++] = *ptr;
	if((*ptr & 0x80) != 0) {
		ptr++;
		while((*ptr & 0xC0) == 0x80)
			Data()[m_Size++] = *ptr++;
	}

	Data()[m_Size] = 0;
	++m_Length;
}

}
