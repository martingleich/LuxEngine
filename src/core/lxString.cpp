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

const StringView StringView::EMPTY = StringView("", 0);

template <typename AddResultT>
static int BasicSplit(AddResultT&& outputter, StringView input, StringView split, int maxCount, bool ignoreEmpty)
{
	if(maxCount == 0)
		return 0;
	if(maxCount < 0)
		maxCount = std::numeric_limits<int>::max();

	int count = 0;
	const char* inCur = input.Data();
	const char* inEnd = inCur + input.Size();
	const char* splitCur = inCur;
	int splitSize = 0;
	if(split.Size() == 1) {
		while(inCur+1 <= inEnd) {
			if(*inCur == *split.Data()) {
				if(!(ignoreEmpty && splitSize == 0)) {
					outputter(splitCur, splitSize);
					++count;
				}
				++inCur;
				splitCur = inCur;
				splitSize = 0;
				if(count == maxCount)
					break;
			} else {
				++splitSize;
				++inCur;
			}
		}
	} else {
		while(inCur+split.Size() <= inEnd) {
			if(memcmp(inCur, split.Data(), split.Size()) == 0) {
				if(!(ignoreEmpty && splitSize == 0)) {
					outputter(splitCur, splitSize);
					++count;
					if(count == maxCount)
						break;
				}
				splitCur = inCur + split.Size();
				splitSize = 0;
			} else {
				++splitSize;
				++inCur;
			}
		}
	}

	if(!(ignoreEmpty && splitSize == 0) && count != maxCount) {
		outputter(splitCur, splitSize);
		++count;
	}
	return count;
}

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

bool StringView::StartsWith(const StringView& data, EStringCompare compare) const
{
	if(data.IsEmpty())
		return true;
	if(data.Size() > Size())
		return false;
	return StringView(Data(), data.Size()).Equal(data, compare);
}

bool StringView::EndsWith(const StringView& data, EStringCompare compare) const
{
	if(data.IsEmpty())
		return true;
	if(data.Size() > Size())
		return false;
	return StringView(Data()+Size()-data.Size(), data.Size()).Equal(data, compare);
}

int StringView::Find(const StringView& search) const
{
	if(search.IsEmpty())
		return 0;
	if(search.Size() > Size())
		return -1;

	const char* cur = Data();
	while(cur + search.Size() <= Data() + Size()) {
		if(memcmp(search.Data(), cur, search.Size()) == 0)
			return cur-Data();
		++cur;
	}

	return -1;
}

int StringView::FindReverse(const StringView& search) const
{
	if(search.IsEmpty())
		return Size()-1;
	if(search.Size() > Size())
		return -1;

	const char* cur = Data() + (Size() - search.Size());
	while(cur >= Data()) {
		if(memcmp(search.Data(), cur, search.Size()) == 0)
			return cur-Data();
		--cur;
	}

	return -1;
}

EStringClassFlag StringView::Classify() const
{
	int alphaCount = 0;
	int digitCount = 0;
	int spaceCount = 0;
	int upperCount = 0;
	int lowerCount = 0;
	int count = 0;
	for(auto c : CodePoints()) {
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

String::String(const char* data, int size) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	if(!data || !size) {
		Reserve(1);
		Data()[0] = 0;
		return;
	}

	if(size < 0)
		size = (int)strlen(data);

	Reserve(size);
	memcpy(Data(), data, size);
	Data()[size] = 0;

	m_Size = size;
}

String::String(const String& other) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	Reserve(other.m_Size);
	memcpy(Data(), other.Data(), other.m_Size + 1);

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

	memcpy(newData, Data(), m_Size + 1);

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
	memcpy(Data(), other.Data(), other.m_Size + 1);
	m_Size = other.m_Size;

	return *this;
}

String& String::operator=(const char* other)
{
	if(Data() == other)
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

void String::Insert(int pos, const StringView& other)
{
	if(other.IsEmpty())
		return;
	int newSize = Size() + other.Size();
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	memmove(data + pos + other.Size(), data + pos, Size() - pos + 1);

	// Place the insertion string.
	memcpy(data + pos, other.Data(), other.Size());

	m_Size = newSize;
}

void String::InsertCodePoint(int pos, u32 codepoint)
{
	char buffer[4];
	int bytes = CodePointToUTF8(codepoint, buffer);
	Insert(pos, StringView(buffer, bytes));
}


String& String::AppendCodePoint(const char* codepoint)
{
	PushCodePoint(codepoint);
	return *this;
}

void String::Resize(int newSize, const StringView& filler)
{
	int curSize = Size();
	if(newSize == 0) {
		m_Size = 0;
	} else if(newSize <= curSize) {
		const char* data = Data();
		const char* ptr = data + m_Size;
		while(curSize > newSize) {
			--ptr;
			--curSize;
		}

		m_Size -= static_cast<int>((data + m_Size) - ptr);
	} else {
		if(filler.IsEmpty())
			throw GenericInvalidArgumentException("filler", "Must not be empty");

		int fillerSize = filler.Size();
		int addSize = static_cast<int>(newSize - curSize);

		int elemCount = addSize / fillerSize;
		int remCount = 0;
		if(addSize%fillerSize != 0)
			remCount = addSize%fillerSize;

		Reserve(newSize);
		if(filler.Size() != 1) {
			char* cur = Data() + m_Size;
			// Copy whole filler strings.
			for(int i = 0; i < elemCount; ++i) {
				memcpy(cur, filler.Data(), filler.Size());
				cur += filler.Size();
			}
			// Copy the last partial string.
			memcpy(cur, filler.Data(), remCount);
		} else {
			char* cur = Data() + m_Size;
			memset(cur, *filler.Data(), elemCount);
		}

		m_Size = newSize;
	}

	Data()[m_Size] = 0;
}

String& String::Clear()
{
	Data()[0] = 0;
	m_Size = 0;
	return *this;
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

int String::Replace(const StringView& replace, const StringView& search, int first, int size)
{
	if(first < 0)
		first = 0;
	if(size < 0)
		size = Size()-first;

	int count = 0;
	int id = StringView(Data() + first, size).Find(search);
	while(id >= 0) {
		++count;
		first = ReplaceRange(replace, first + id, search.Size());
		size = size - replace.Size();
		id = StringView(Data() + first, size).Find(search);
	}

	return count;
}

int String::ReplaceRange(const StringView& replace, int first, int size)
{
	if(first < 0)
		first = 0;
	if(size < 0)
		size = Size()-first;

	if(m_Size + replace.Size() < size)
		throw GenericInvalidArgumentException("size", "size must not be to large.");

	int newSize = m_Size - size + replace.Size();
	Reserve(newSize);

	char* data = Data();
	int restSize = m_Size - first - size;
	memmove(data + first + replace.Size(), data + first + size, restSize + 1); // Including NUL
	memcpy(data + first, replace.Data(), replace.Size());

	m_Size = newSize;

	return first + replace.Size();
}

int String::Pop(int size)
{
	if(size <= 0)
		return 0;
	int newSize = m_Size - size;
	if(newSize < 0)
		newSize = 0;
	Data()[newSize] = 0;
	int removed = m_Size - newSize;
	m_Size = newSize;
	return removed;
}

void String::Remove(int pos, int size)
{
	if(size < 0)
		size = 1;
	if(pos + size > m_Size)
		throw GenericInvalidArgumentException("size", "size must not be to large.");

	int newSize = m_Size - size;

	char* data = Data();
	int restSize = m_Size - pos - size;
	memmove(data + pos, data + pos + size, restSize + 1); // Including NUL

	m_Size = newSize;
}

String& String::RStrip(int end)
{
	if(end < 0)
		end = Size();

	char* data = Data();

	// Find last element.
	ConstUTF8Iterator begin = Data();
	ConstUTF8Iterator last = ConstUTF8Iterator(data + end) - 1;;
	ConstUTF8Iterator cur = last;
	int lastSize = (data+end)-cur.Pointer();
	int removed = 0;
	while(cur.Pointer() != begin && IsSpace(*cur)) {
		--cur;
		++removed;
	}

	if(cur == begin)
		m_Size = 0;
	else
		m_Size = static_cast<int>(cur.Pointer() - data) + lastSize;

	data[m_Size] = 0;

	return *this;
}

String& String::LStrip(int first)
{
	if(first < 0)
		first = 0;

	auto begin = ConstUTF8Iterator(Data()+first);
	auto cur = begin;
	while(cur.Pointer() != End() && IsSpace(*cur))
		++cur;

	auto removed = static_cast<int>(cur.Pointer() - begin.Pointer());
	Remove(first, removed);
	return *this;
}

String& String::Strip(int first, int end)
{
	RStrip(end);
	LStrip(first);
	return *this;
}

int String::Split(StringView split, String* outArray, int maxCount, bool ignoreEmpty) const
{
	return BasicSplit([&outArray](const char* data, int size) { *outArray++ = StringView(data, size); },
		(StringView)*this, split, maxCount, ignoreEmpty);
}

Array<String> String::Split(StringView split, bool ignoreEmpty) const
{
	Array<String> out;
	BasicSplit([&out](const char* data, int size) { out.EmplaceBack(data, size); },
		(StringView)*this, split, -1, ignoreEmpty);
	return out;
}

String String::GetLower() const
{
	String out;
	out.Reserve(Size());

	for(auto c : CodePoints()) {
		c = ToLowerChar(c);
		out.AppendCodePoint(c);
	}

	return out;
}

String String::GetUpper() const
{
	String out;
	out.Reserve(Size());

	for(auto c : CodePoints()) {
		c = ToUpperChar(c);
		out.AppendCodePoint(c);
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

void String::PushCodePoint(const char* ptr)
{
	if(m_Size + 4 >= GetAllocated()) // Maximal 4 utf-8 bytes
		Reserve(GetAllocated() * 2 + 4);

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
