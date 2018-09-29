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
	static const Type t(new TypeInfoTemplate<lux::core::String>("String"));
	return t;
}
} // namespace Types

bool StringView::Equal(const StringView& other, EStringCompare compare) const
{
	if(Size() != other.Size())
		return false;

	switch(compare) {
	case EStringCompare::CaseSensitive:
		return (memcmp(Data(), other.Data(), Size()) == 0);
	case EStringCompare::CaseInsensitive:
	{
		const char* a = Data();
		const char* b = other.Data();
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
	}
	return false;
}

bool StringView::Smaller(const StringView& other, EStringCompare compare) const
{
	switch(compare) {
	case EStringCompare::CaseSensitive:
	{
		auto size = other.Size();
		auto s = Size() < size ? Size() : size;
		return (memcmp(Data(), other.Data(), s) < 0);
	}
	case EStringCompare::CaseInsensitive:
	{
		const char* a = Data();
		const char* b = other.Data();
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
	}
	return false;
}

bool StringView::StartsWith(const StringView& data, ConstByteIterator first, EStringCompare compare) const
{
	if(first == nullptr)
		first = Data();

	if(data.Data()[0] == 0)
		return true;

	switch(compare) {
	case EStringCompare::CaseSensitive:
	{
		auto strCur = first;
		for(auto it = data.Data(); *it != 0; ++it, ++strCur) {
			if(*it != *strCur)
				return false;
		}
		return true;
	}
	case EStringCompare::CaseInsensitive:
	{
		ConstUTF8Iterator strCur = first;
		for(auto c : *this) {
			if(!IsEqualCaseInsensitive(c, *strCur))
				return false;
			++strCur;
		}
		return true;
	}
	}

	return true;
}

bool StringView::EndsWith(const StringView& data, ConstByteIterator end, EStringCompare compare) const
{
	if(end == nullptr)
		end = End();

	if(data.Data()[0] == 0)
		return true;

	switch(compare) {
	case EStringCompare::CaseSensitive:
	{
		auto dataCur = data.Data() + data.Size() - 1;
		auto strCur = end - 1;
		int i = 0;
		for(auto it = dataCur; i < data.Size(); --it, --strCur, ++i) {
			if(*it != *strCur)
				break;
		}
		return (i == data.Size());
	}
	case EStringCompare::CaseInsensitive:
	{
		ConstUTF8Iterator dataCur = data.Data() + data.Size() - 1;
		ConstUTF8Iterator strCur = end - 1;
		int i = 0;
		for(auto it = dataCur; i < data.Size(); --it, --strCur) {
			if(!IsEqualCaseInsensitive(*it, *strCur))
				break;
			i = static_cast<int>((end - 1) - (const char*)strCur);
		}
		return (i == data.Size());
	}
	}

	return false;
}

StringView::ConstIterator StringView::Find(const StringView& search, ConstByteIterator first, ConstByteIterator end) const
{
	if(first == nullptr)
		first = First();
	if(end == nullptr)
		end = End();

	if(search.Size() == 0)
		return end;

	const char* searchFirst = search.Data();
	while(first + search.Size() <= end) {
		if(memcmp(searchFirst, first, search.Size()) == 0)
			return first;
		++first;
	}

	return end;
}

StringView::ConstIterator StringView::FindReverse(const StringView& search, ConstByteIterator first, ConstByteIterator end) const
{
	if(first == nullptr)
		first = First();
	if(end == nullptr)
		end = End();

	if(first == end)
		return end;

	if(search.Size() == 0)
		return end;

	ConstByteIterator cur = end;
	const char* searchFirst = search.Data();

	while(cur - search.Size() >= first) {
		if(memcmp(cur - search.Size(), searchFirst, search.Size()) == 0)
			return ConstIterator(cur - search.Size(), Data());
		--cur;
	}

	return end;
}

EStringClassFlag StringView::Classify() const
{
	int alphaCount = 0;
	int digitCount = 0;
	int spaceCount = 0;
	int upperCount = 0;
	int lowerCount = 0;
	int count = 0;
	for(auto c : *this) {
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
		++count;
	}

	EStringClassFlag out = (EStringClassFlag)0;
	if(Size() == 0)
		out |= EStringClassFlag::Empty;

	if(lowerCount == alphaCount && alphaCount > 0)
		out |= EStringClassFlag::Lower;
	else if(upperCount == alphaCount && alphaCount > 0)
		out |= EStringClassFlag::Upper;

	if(alphaCount == count && alphaCount > 0)
		out |= EStringClassFlag::Alpha;
	else if(digitCount == count && digitCount > 0)
		out |= EStringClassFlag::Digit;
	else if(alphaCount + digitCount == count && alphaCount > 0 && digitCount > 0)
		out |= EStringClassFlag::AlphaNum;
	else if(spaceCount == count && spaceCount > 0)
		out |= EStringClassFlag::Space;

	return out;
}

bool StringView::IsWhitespace() const
{
	return IsEmpty() || Classify() == EStringClassFlag::Space;
}

////////////////////////////////////////////////////////////////////////////

const String String::EMPTY = String();

String::String() :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
}

String::String(const char* data, int length) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	if(!data || !length) {
		Reserve(1);
		Data()[0] = 0;
		return;
	}

	int size;
	if(length < 0) {
		size = (int)strlen(data);
	} else {
		const char* cur = data;
		int count = 0;
		while(count < length && AdvanceCursorUTF8(cur))
			++count;
		size = static_cast<int>(cur - data);
	}

	Reserve(size);
	memcpy(Data(), data, size);
	Data()[size] = 0;

	m_Size = size;
}

String::String(ConstByteIterator first, ConstByteIterator end) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	Append(first, end);
}

String::String(const String& other) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	Reserve(other.m_Size);
	memcpy(Data(), other.Data_c(), other.m_Size + 1);

	m_Size = other.m_Size;
}

String::String(String&& old) :
	m_Data(old.m_Data),
	m_Allocated(old.m_Allocated),
	m_Size(old.m_Size)
{
	old.m_Data.ptr = nullptr;
	old.m_Allocated = 0;
	old.m_Size = 0;
}

String::~String()
{
	if(!IsShortString())
		LUX_FREE_RAW(m_Data.ptr);
}

String String::Copy()
{
	return String(*this);
}

void String::Reserve(int size)
{
	int alloc = GetAllocated();
	int newAlloc = size + 1;
	if(newAlloc <= alloc)
		return;

	char* newData;
	bool willBeShort = (newAlloc <= MAX_SHORT_BYTES);
	if(willBeShort)
		newData = m_Data.raw;
	else
		newData = (char*)LUX_NEW_RAW(newAlloc);

	memcpy(newData, Data_c(), m_Size + 1);

	if(!IsShortString())
		LUX_FREE_RAW(m_Data.ptr);
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

	int size = (int)strlen(other);
	Reserve(size);
	memcpy(Data(), other, size + 1);
	m_Size = size;

	return *this;
}

String& String::operator=(String&& old)
{
	this->~String();
	m_Data = old.m_Data;
	m_Allocated = old.m_Allocated;
	m_Size = old.m_Size;
	old.m_Data.ptr = nullptr;
	old.m_Allocated = 0;
	old.m_Size = 0;

	return *this;
}

String& String::operator+=(const StringView& str)
{
	return Append(str);
}

String::ConstIterator String::Insert(ConstByteIterator pos, const StringView& other, int count)
{
	int size;
	if(count < 0) {
		size = other.Size();
	} else {
		const char* cur = other.Data();
		int c = 0;
		while(c < count && AdvanceCursorUTF8(cur))
			++c;
		size = static_cast<int>(cur - other.Data());
	}
	if(size == 0)
		return pos;
	int newSize = m_Size + size;
	int pos_off = static_cast<int>(pos - Data());
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	memmove(data + pos_off + size, data + pos_off, m_Size - pos_off + 1);

	// Place the insertion string.
	memcpy(data + pos_off, other.Data(), size);

	m_Size += size;

	return ConstIterator(data + pos_off + size, Data_c());
}

String::ConstIterator String::Insert(ConstByteIterator pos, ConstByteIterator first, ConstByteIterator end)
{
	// This assumes, string::ConstByteIterator point to a continues block of memory until end.
	// This is always the case for string::ConstByteIterator.

	int size = static_cast<int>(end - first);

	int newSize = m_Size + size;
	int pos_off = static_cast<int>(pos - Data());
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	memmove(data + pos_off + size, data + pos_off, m_Size - pos_off + 1);

	// Place the insertion string.
	memcpy(data + pos_off, first, size);

	m_Size += size;

	return ConstIterator(data + pos_off + size, Data_c());
}

String& String::AppendRaw(const char* data, int bytes)
{
	Reserve(m_Size + bytes);
	auto cur = Data() + m_Size;

	// Copy and zero terminate string
	memcpy(cur, data, bytes);
	cur[bytes] = 0;

	// Calculate length and size
	m_Size += bytes;

	return *this;
}

String& String::Append(const StringView& other, int count)
{
	Insert(End(), other, count);
	return *this;
}

String& String::Append(ConstByteIterator first, ConstByteIterator end)
{
	Insert(End(), first, end);
	return *this;
}

String& String::Append(ConstByteIterator character)
{
	PushCharacter(character);
	return *this;
}

String& String::Append(u32 character)
{
	u8 buffer[6];
	u8* end = CodePointToUTF8(character, buffer);
	auto size = static_cast<int>(end - buffer);
	Reserve(m_Size + size);

	memcpy(Data() + m_Size, buffer, size);
	m_Size += size;
	Data()[m_Size] = 0;

	return *this;
}

void String::Resize(int newLength, const StringView& filler)
{
	auto curLength = End() - First();
	if(newLength == 0) {
		m_Size = 0;
	} else if(newLength <= curLength) {
		const char* data = Data();
		const char* ptr = data + m_Size;
		while(curLength > newLength) {
			RetractCursorUTF8(ptr);
			--curLength;
		}

		m_Size -= static_cast<int>((data + m_Size) - ptr);
	} else {
		if(filler.IsEmpty())
			throw GenericInvalidArgumentException("filler", "Must not be empty");

		int fillerLength = filler.Size() == 1 ? 1 : StringLengthUTF8(filler.Data());
		int addLength = static_cast<int>(newLength - curLength);

		int elemCount = addLength / fillerLength;
		int addBytes = elemCount*filler.Size();
		int remCount = 0;
		int neededBytes = elemCount * filler.Size();
		if(addLength%fillerLength != 0) {
			neededBytes += filler.Size();
			remCount = addLength%fillerLength;
		}

		Reserve(m_Size + neededBytes);
		if(filler.Size() != 1) {
			char* cur = Data() + m_Size;
			// Copy whole filler strings.
			for(int i = 0; i < elemCount; ++i) {
				memcpy(cur, filler.Data(), filler.Size());
				cur += filler.Size();
			}
			// Copy the last partial string.
			const char* fillCur = filler.Data();
			int i = 0;
			while(i < remCount) {
				*cur++ = *fillCur;
				if((*fillCur & 0xC0) != 0x80)
					++i;
				++addBytes;
			}
		} else {
			char* cur = Data() + m_Size;
			memset(cur, *filler.Data(), elemCount);
		}

		m_Size += addBytes;
	}

	Data()[m_Size] = 0;
}

String& String::Clear()
{
	Data()[0] = 0;
	m_Size = 0;
	return *this;
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
		throw GenericInvalidArgumentException("byte", "byte must not be zero.");

	Data()[m_Size] = byte;
	Data()[m_Size + 1] = 0;

	++m_Size;
}

int String::Replace(const StringView& replace, const StringView& search, ConstByteIterator first, ConstByteIterator end)
{
	if(end == nullptr)
		end = End();

	if(first == nullptr)
		first = First();

	int count = 0;
	int length = StringLengthUTF8(search.Data());
	ConstByteIterator it = Find(search, first, end);
	while(it != end) {
		int endOffset = static_cast<int>(end - Data_c());
		it = ReplaceRange(replace, it, length);
		end = ConstIterator(Data_c() + endOffset, Data_c());
		it = Find(search, it, end);

		++count;
	}

	return count;
}

String::ConstIterator String::ReplaceRange(const StringView& replace, ConstByteIterator rangeFirst, ConstByteIterator rangeEnd)
{
	if(rangeEnd == nullptr)
		rangeEnd = End();

	return ReplaceRange(replace, rangeFirst, IteratorDistance(rangeFirst, rangeEnd));
}

String::ConstIterator String::ReplaceRange(const StringView& replace, ConstByteIterator rangeFirst, int count)
{
	int replacedSize = static_cast<int>((ConstIterator(rangeFirst) + count).Pointer() - rangeFirst);

	if(m_Size + replace.Size() < replacedSize)
		throw GenericInvalidArgumentException("count", "count must not be to large.");

	int newSize = m_Size - replacedSize + replace.Size();
	int replaceOffset = static_cast<int>(rangeFirst - Data_c());
	Reserve(newSize);

	char* data = Data();
	int restSize = m_Size - replaceOffset - replacedSize;
	memmove(data + replaceOffset + replace.Size(), data + replaceOffset + replacedSize, restSize + 1); // Including NUL
	memcpy(data + replaceOffset, replace.Data(), replace.Size());

	m_Size = newSize;

	return ConstIterator(data + replaceOffset + replacedSize, data);
}

int String::Pop(int count)
{
	ConstIterator it = Last();
	int oldCount = count;
	while(it != First() && count > 0) {
		--it;
		--count;
	}

	const char* last = (it + 1).Pointer();
	int newSize = static_cast<int>(last - Data());
	if(it == First() && count > 0) {
		newSize = 0;
		--count;
	}

	Data()[newSize] = 0;
	m_Size = newSize;
	int removed = oldCount - count;
	return removed;
}

String String::SubString(ConstByteIterator first, int count) const
{
	return String(first, count);
}

String String::SubString(ConstByteIterator first, ConstByteIterator end) const
{
	return String(first, end);
}

String::ConstIterator String::Remove(ConstByteIterator pos, int count)
{
	int removeSize = static_cast<int>((ConstIterator(pos) + count).Pointer() - pos);

	if(removeSize > m_Size)
		throw GenericInvalidArgumentException("count", "count must not be to large.");

	int newSize = m_Size - removeSize;

	char* data = Data();
	int removeOffset = static_cast<int>(pos - data);
	int restSize = m_Size - removeOffset - removeSize;
	memmove(data + removeOffset, data + removeOffset + removeSize, restSize + 1); // Including NUL

	m_Size = newSize;
	return ConstIterator(data + removeOffset, Data());
}

String::ConstIterator String::Remove(ConstByteIterator from, ConstByteIterator to)
{
	return Remove(from, core::IteratorDistance(from, to));
}

String& String::RStrip(ConstByteIterator end)
{
	if(end == nullptr)
		end = End();

	if(end == First())
		return *this;

	char* data = Data();

	ConstIterator last = ConstIterator(end) - 1;
	int lastSize = core::IteratorDistance(last.Pointer(), end);
	ConstIterator begin = Begin();
	int removed = 0;
	while(last != begin && IsSpace(*last)) {
		--last;
		++removed;
	}

	if(last == begin)
		m_Size = 0;
	else
		m_Size = static_cast<int>(last.Pointer() - data) + lastSize;

	data[m_Size] = 0;

	return *this;
}

String& String::LStrip(ConstByteIterator first)
{
	if(first == nullptr)
		first = First();

	auto offset = static_cast<int>(first - Data_c());
	auto end = End();
	int count = 0;
	while(first != end && IsSpace(*first)) {
		++first;
		++count;
	}

	char* base = Data() + offset;
	auto removed = static_cast<int>(first - base);
	memmove(base, first, m_Size - offset - removed);
	m_Size -= removed;

	return *this;
}

String& String::Strip(ConstByteIterator first, ConstByteIterator end)
{
	RStrip(end);
	LStrip(first);
	return *this;
}

int String::Split(u32 ch, String* outArray, int maxCount, bool ignoreEmpty) const
{
	if(maxCount == 0)
		return 0;

	String* cur = outArray;
	int count = 1;
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
	for(auto c : *this) {
		if(c == ch) {
			if(!(ignoreEmpty && buffer.IsEmpty()))
				out.PushBack(std::move(buffer));
			buffer.Clear();
		} else {
			buffer.Append(c);
		}
	}

	if(!(ignoreEmpty && buffer.IsEmpty()))
		out.PushBack(std::move(buffer));

	return out;
}

String String::GetLower() const
{
	String out;
	out.Reserve(Size());

	for(auto c : *this) {
		c = ToLowerChar(c);
		out.Append(c);
	}

	return out;
}

String String::GetUpper() const
{
	String out;
	out.Reserve(Size());

	for(auto c : *this) {
		c = ToUpperChar(c);
		out.Append(c);
	}

	return out;
}

bool String::IsShortString() const
{
	return m_Allocated <= MAX_SHORT_BYTES;
}

int String::GetAllocated() const
{
	return m_Allocated;
}

void String::SetAllocated(int a)
{
	m_Allocated = a;
}

void String::PushCharacter(const char* ptr)
{
	if(m_Size + 6 >= GetAllocated()) // Maximal 6 utf-8 bytes
		Reserve(GetAllocated() * 2 + 6);

	if(*ptr == 0)
		throw GenericInvalidArgumentException("byte", "byte must not be null");

	Data()[m_Size++] = *ptr;
	if((*ptr & 0x80) != 0) {
		ptr++;
		while((*ptr & 0xC0) == 0x80)
			Data()[m_Size++] = *ptr++;
	}

	Data()[m_Size] = 0;
}

} // namespace core
} // namespace lux
