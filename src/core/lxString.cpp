#include "core/lxString.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxArray.h"

namespace lux
{

namespace core
{
namespace Types
{
Type String()
{
	static const Type t(new TypeInfoTemplate<lux::core::String>("string"));
	return t;
}
} // namespace Types

const String String::EMPTY = String();

String::String() :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0),
	m_Length(0)
{
}

String::String(const char* data, size_t length) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0),
	m_Length(0)
{
	if(!data || !length) {
		Reserve(1);
		Data()[0] = 0;
		return;
	}

	size_t size;
	if(length == std::numeric_limits<size_t>::max()) {
		length = StringLengthUTF8(data, &size);
	} else {
		const char* cur = data;
		size_t count = 0;
		while(count < length && AdvanceCursorUTF8(cur))
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

String::String(ConstIterator first, ConstIterator end) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0),
	m_Length(0)
{
	Append(first, end);
}

String::String(const String& other) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0),
	m_Length(0)
{
	Reserve(other.m_Size);
	memcpy(Data(), other.Data_c(), other.m_Size + 1);

	m_Length = other.m_Length;
	m_Size = other.m_Size;
}

String::String(String&& old) :
	m_Data(old.m_Data),
	m_Allocated(old.m_Allocated),
	m_Size(old.m_Size),
	m_Length(old.m_Length)
{
	old.m_Data.ptr = nullptr;
	old.m_Allocated = 0;
	old.m_Size = 0;
	old.m_Length = 0;
}

String::~String()
{
	if(!IsShortString())
		delete[] m_Data.ptr;
}

String String::Copy()
{
	return String(*this);
}

void String::Reserve(size_t size)
{
	size_t alloc = GetAllocated();
	size_t newAlloc = size + 1;
	if(newAlloc <= alloc)
		return;

	char* newData;
	bool willBeShort = (newAlloc <= MAX_SHORT_BYTES);
	if(willBeShort)
		newData = m_Data.raw;
	else
		newData = new char[newAlloc];

	memcpy(newData, Data_c(), m_Size + 1);

	if(!IsShortString())
		delete[] m_Data.ptr;
	if(!willBeShort)
		m_Data.ptr = newData;

	SetAllocated(newAlloc);
}

String& String::operator=(const String& other)
{
	if(this == &other)
		return *this;

	Reserve(other.m_Size);
	memcpy(Data(), other.Data_c(), other.m_Size + 1);
	m_Size = other.m_Size;
	m_Length = other.m_Length;

	return *this;
}

String& String::operator=(const char* other)
{
	if(Data_c() == other)
		return *this;
	if(!other) {
		Clear();
		return *this;
	}

	size_t size;
	size_t length = StringLengthUTF8(other, &size);
	Reserve(size);
	memcpy(Data(), other, size + 1);
	m_Size = size;
	m_Length = length;

	return *this;
}

String& String::operator=(String&& old)
{
	this->~String();
	m_Data = old.m_Data;
	m_Allocated = old.m_Allocated;
	m_Size = old.m_Size;
	m_Length = old.m_Length;
	old.m_Data.ptr = nullptr;
	old.m_Allocated = 0;
	old.m_Length = 0;
	old.m_Size = 0;

	return *this;
}

String& String::operator+=(const StringType& str)
{
	return Append(str);
}
String String::operator+(const StringType& str) const
{
	String n(*this);
	n += str;
	return n;
}

bool String::operator==(const StringType& other) const
{
	other.EnsureSize();
	if(m_Size != other.size)
		return false;

	return (memcmp(Data(), other.data, m_Size) == 0);
}

bool String::operator<(const StringType& other) const
{
	other.EnsureSize();
	size_t s = m_Size < other.size ? m_Size : other.size;
	return (memcmp(Data(), other.data, s) < 0);
}

bool String::operator!=(const StringType& other) const
{
	return !(*this == other);
}

bool String::Equal(const StringType& other) const
{
	return (*this == other);
}

bool String::EqualCaseInsensitive(const StringType& other) const
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
		ac = AdvanceCursorUTF8(a);
		bc = AdvanceCursorUTF8(b);

		if(!IsEqualCaseInsensitive(ac, bc))
			return false;
	} while(ac && bc);

	return (ac == bc);
}

bool String::SmallerCaseInsensitive(const StringType& other) const
{
	const char* a = Data();
	const char* b = other.data;
	u32 ac;
	u32 bc;
	do {
		ac = AdvanceCursorUTF8(a);
		bc = AdvanceCursorUTF8(b);

		u32 iac = ToLowerChar(ac);
		u32 ibc = ToLowerChar(bc);
		if(iac < ibc)
			return true;
		if(iac > ibc)
			return false;
	} while(ac && bc);

	return (ac < bc);
}

String::ConstIterator String::Insert(ConstIterator pos, const StringType& other, size_t count)
{
	lxAssert(pos.m_First == Data());

	other.EnsureSize();
	size_t size;
	if(count == std::numeric_limits<size_t>::max()) {
		count = StringLengthUTF8(other.data);
		size = other.size;
	} else {
		const char* cur = other.data;
		size_t c = 0;
		while(c < count && AdvanceCursorUTF8(cur))
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

String::ConstIterator String::Insert(ConstIterator pos, ConstIterator first, ConstIterator end)
{
	// This assumes, string::ConstIterator point to a continues block of memory until end.
	// This is always the case for string::ConstIterator.

	size_t size = end.Pointer() - first.Pointer();
	size_t count = IteratorDistance(first, end);

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

String& String::AppendRaw(const char* data, size_t bytes)
{
	Reserve(m_Size + bytes);
	memcpy(Data() + m_Size, data, bytes);
	m_Length += StringLengthUTF8(data);
	m_Size += bytes;
	Data()[m_Size] = 0;
	return *this;
}

String& String::Append(const StringType& other, size_t count)
{
	Insert(End(), other, count);
	return *this;
}

String& String::Append(ConstIterator first, ConstIterator end)
{
	Insert(End(), first, end);
	return *this;
}

String& String::Append(ConstIterator character)
{
	PushCharacter(character.Pointer());
	return *this;
}

String& String::Append(u32 character)
{
	u8 buffer[6];
	u8* end = CodePointToUTF8(character, buffer);
	const size_t size = end - buffer;
	Reserve(m_Size + size);

	memcpy(Data() + m_Size, buffer, size);
	m_Size += size;
	Data()[m_Size] = 0;

	++m_Length;

	return *this;
}

void String::Resize(size_t newLength, const StringType& filler)
{
	if(newLength == 0) {
		m_Length = 0;
		m_Size = 0;
	} else if(newLength <= m_Length) {
		const char* data = Data();
		const char* ptr = data + m_Size;
		while(m_Length > newLength) {
			RetractCursorUTF8(ptr);
			--m_Length;
		}

		m_Size -= (data + m_Size) - ptr;
	} else {
		filler.EnsureSize();
		if(filler.size == 0)
			throw InvalidArgumentException("filler", "length(filler) > 0");

		size_t fillerLength = filler.size == 1 ? 1 : StringLengthUTF8(filler.data);
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

String& String::Clear()
{
	Data()[0] = 0;
	m_Length = 0;
	m_Size = 0;
	return *this;
}

size_t String::Size() const
{
	return m_Size;
}

size_t String::Length() const
{
	return m_Length;
}

bool String::IsEmpty() const
{
	return (Size() == 0);
}

const char* String::Data_c() const
{
	return Data();
}

const char* String::Data() const
{
	if(IsShortString() || !m_Data.ptr)
		return m_Data.raw;
	else
		return m_Data.ptr;
}

char* String::Data()
{
	if(IsShortString() || !m_Data.ptr)
		return m_Data.raw;
	else
		return m_Data.ptr;
}

void String::PushByte(u8 byte)
{
	if(m_Size + 1 >= GetAllocated())
		Reserve(GetAllocated() * 2 + 1);

	if(byte == 0)
		throw InvalidArgumentException("byte", "byte must not be null");

	Data()[m_Size] = byte;
	Data()[m_Size + 1] = 0;

	++m_Size;
	if((byte & 0xC0) != 0x80) // Bytes starting with b10 are continuation bytes.
		++m_Length;
}

String::ConstIterator String::Begin() const
{
	return ConstIterator(Data_c() - 1, Data_c());
}

String::ConstIterator String::First() const
{
	return ConstIterator(Data_c(), Data_c());
}

String::ConstIterator String::Last() const
{
	if(m_Size > 0)
		return End() - 1;
	else
		return End();
}

String::ConstIterator String::End() const
{
	return ConstIterator(Data_c() + m_Size, Data_c());
}

bool String::StartsWith(const StringType& data, ConstIterator first) const
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

bool String::EndsWith(const StringType& data, ConstIterator end) const
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

size_t String::Replace(const StringType& replace, const StringType& search, ConstIterator first, ConstIterator end)
{
	if(end == ConstIterator::Invalid())
		end = End();

	if(first == ConstIterator::Invalid())
		first = First();

	size_t count = 0;
	size_t length = StringLengthUTF8(search.data);
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

String::ConstIterator String::ReplaceRange(const StringType& replace, ConstIterator rangeFirst, ConstIterator rangeEnd)
{
	if(rangeEnd == ConstIterator::Invalid())
		rangeEnd = End();

	return ReplaceRange(replace, rangeFirst, IteratorDistance(rangeFirst, rangeEnd));
}

String::ConstIterator String::ReplaceRange(const StringType& replace, ConstIterator rangeFirst, size_t count)
{
	lxAssert(rangeFirst.m_First == Data());

	replace.EnsureSize();
	size_t replacedSize = (rangeFirst + count).Pointer() - rangeFirst.Pointer();

	if(m_Size + replace.size < replacedSize)
		throw InvalidArgumentException("count", "count must not be to large.");

	size_t newSize = m_Size - replacedSize + replace.size;
	size_t replaceOffset = rangeFirst.Pointer() - Data_c();
	Reserve(newSize);

	char* data = Data();
	size_t restSize = m_Size - replaceOffset - replacedSize;
	memmove(data + replaceOffset + replace.size, data + replaceOffset + replacedSize, restSize + 1); // Including NUL
	memcpy(data + replaceOffset, replace.data, replace.size);

	m_Size = newSize;
	m_Length = m_Length - count + StringLengthUTF8(replace.data);

	return ConstIterator(data + replaceOffset + replacedSize, data);
}

String::ConstIterator String::Find(const StringType& search, ConstIterator first, ConstIterator end) const
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

String::ConstIterator String::FindReverse(const StringType& search, ConstIterator first, ConstIterator end) const
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

String String::SubString(ConstIterator first, size_t count) const
{
	lxAssert(first.m_First == Data());

	return String(first.Pointer(), count);
}

String String::SubString(ConstIterator first, ConstIterator end) const
{
	lxAssert(first.m_First == Data());
	lxAssert(end.m_First == Data());

	String out;
	ConstIterator it = first;
	while(it != end) {
		out.Append(it);
		++it;
	}

	return out;
}

size_t String::Pop(size_t count)
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

String::ConstIterator String::Remove(ConstIterator pos, size_t count)
{
	lxAssert(pos.m_First == Data());

	size_t removeSize = (pos + count).Pointer() - pos.Pointer();

	if(removeSize > m_Size)
		throw InvalidArgumentException("count", "count must not be to large.");

	size_t newSize = m_Size - removeSize;

	char* data = Data();
	size_t removeOffset = pos.Pointer() - data;
	size_t restSize = m_Size - removeOffset - removeSize;
	memmove(data + removeOffset, data + removeOffset + removeSize, restSize + 1); // Including NUL

	m_Size = newSize;
	m_Length = m_Length - count;

	return ConstIterator(data + removeOffset, Data());
}

String::ConstIterator String::Remove(ConstIterator from, ConstIterator to)
{
	return Remove(from, IteratorDistance(from, to));
}

String& String::RStrip(ConstIterator end)
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
	while(last != begin && IsSpace(*last)) {
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

String& String::LStrip(ConstIterator first)
{
	if(first == ConstIterator::Invalid())
		first = First();

	lxAssert(first.m_First == Data());

	size_t offset = first.Pointer() - Data_c();
	auto end = End();
	size_t count = 0;
	while(first != end && IsSpace(*first)) {
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

size_t String::Split(u32 ch, String* outArray, size_t maxCount, bool ignoreEmpty) const
{
	if(maxCount == 0)
		return 0;

	String* cur = outArray;
	size_t count = 1;
	cur->Clear();
	for(auto it = First(); it != End(); ++it) {
		if(*it == ch) {
			if(!(ignoreEmpty && cur->IsEmpty())) {
				if(count == maxCount)
					return maxCount;
				++cur;
				++count;
			}
			cur->Clear();
		} else {
			cur->Append(*it);
		}
	}

	return count;
}

Array<String> String::Split(u32 ch, bool ignoreEmpty) const
{
	Array<String> out;
	String buffer;
	for(auto it = First(); it != End(); ++it) {
		if(*it == ch) {
			if(!(ignoreEmpty && buffer.IsEmpty()))
				out.PushBack(std::move(buffer));
			buffer.Clear();
		} else {
			buffer.Append(*it);
		}
	}

	if(!(ignoreEmpty && buffer.IsEmpty()))
		out.PushBack(std::move(buffer));

	return out;
}

EStringClass String::Classify() const
{
	size_t alphaCount = 0;
	size_t digitCount = 0;
	size_t spaceCount = 0;
	size_t upperCount = 0;
	size_t lowerCount = 0;
	for(auto it = First(); it != End(); ++it) {
		uint32_t c = *it;
		if(IsLower(c))
			++lowerCount;
		else if(IsUpper(c))
			++upperCount;

		if(IsAlpha(c))
			++alphaCount;
		else if(IsDigit(c))
			++digitCount;
		else if(IsSpace(c))
			++spaceCount;
	}

	EStringClass out = (EStringClass)0;
	if(m_Length == 0)
		out |= EStringClass::Empty;

	if(lowerCount == alphaCount && alphaCount > 0)
		out |= EStringClass::Lower;
	else if(upperCount == alphaCount && alphaCount > 0)
		out |= EStringClass::Upper;

	if(alphaCount == m_Length && alphaCount > 0)
		out |= EStringClass::Alpha;
	else if(digitCount == m_Length && digitCount > 0)
		out |= EStringClass::Digit;
	else if(alphaCount + digitCount == m_Length && alphaCount > 0 && digitCount > 0)
		out |= EStringClass::AlphaNum;
	else if(spaceCount == m_Length && spaceCount > 0)
		out |= EStringClass::Space;

	return out;
}

bool String::IsWhitespace() const
{
	return IsEmpty() || Classify() == EStringClass::Space;
}

String String::GetLower() const
{
	String out;
	out.Reserve(Size());

	for(auto it = First(); it != End(); ++it) {
		u32 c = ToLowerChar(*it);
		out.Append(c);
	}

	return out;
}

String String::GetUpper() const
{
	String out;
	out.Reserve(Size());

	for(auto it = First(); it != End(); ++it) {
		u32 c = ToUpperChar(*it);
		out.Append(c);
	}

	return out;
}

bool String::IsShortString() const
{
	return m_Allocated <= MAX_SHORT_BYTES;
}

size_t String::GetAllocated() const
{
	return m_Allocated;
}

void String::SetAllocated(size_t a)
{
	m_Allocated = a;
}

void String::PushCharacter(const char* ptr)
{
	if(m_Size + 6 >= GetAllocated()) // Maximal 6 utf-8 bytes
		Reserve(GetAllocated() * 2 + 6);

	if(*ptr == 0)
		throw InvalidArgumentException("byte", "byte must not be null");

	Data()[m_Size++] = *ptr;
	if((*ptr & 0x80) != 0) {
		ptr++;
		while((*ptr & 0xC0) == 0x80)
			Data()[m_Size++] = *ptr++;
	}

	Data()[m_Size] = 0;
	++m_Length;
}

} // namespace core
} // namespace lux
