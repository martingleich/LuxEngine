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
		size = core::SafeCast<int>(std::strlen(data));

	Reserve(size);
	std::memcpy(Data(), data, size);
	Data()[size] = 0;

	m_Size = size;
}

String::String(const String& other) :
	m_Data({nullptr}),
	m_Allocated(0),
	m_Size(0)
{
	Reserve(other.m_Size);
	std::memcpy(Data(), other.Data(), other.m_Size + 1);

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

	std::memcpy(newData, Data(), m_Size + 1);

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
	std::memcpy(Data(), other.Data(), other.m_Size + 1);
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

	int size = core::SafeCast<int>(std::strlen(other));
	Reserve(size);
	std::memcpy(Data(), other, size + 1);
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

void String::Insert(int pos, StringView other)
{
	if(other.IsEmpty())
		return;
	int newSize = Size() + other.Size();
	Reserve(newSize);

	// Move last part of string back, including NUL.
	char* data = Data();
	std::memmove(data + pos + other.Size(), data + pos, Size() - pos + 1);

	// Place the insertion string.
	std::memcpy(data + pos, other.Data(), other.Size());

	m_Size = newSize;
}

void String::InsertCodePoint(int pos, u32 codepoint)
{
	char buffer[4];
	int bytes = CodePointToUTF8(codepoint, buffer);
	Insert(pos, StringView(buffer, bytes));
}

String& String::AppendCodePoint(const void* codepoint)
{
	if(m_Size + 4 >= GetAllocated()) // Maximal 4 utf-8 bytes
		Reserve(GetAllocated() * 2 + 4);

	auto cp = (char*)codepoint;
	auto data = Data();
	data[m_Size++] = *cp;
	if((*cp & 0x80) != 0) {
		cp++;
		while((*cp & 0xC0) == 0x80)
			data[m_Size++] = *cp++;
	}

	data[m_Size] = 0;
	return *this;
}

void String::Resize(int newSize, StringView filler)
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
				std::memcpy(cur, filler.Data(), filler.Size());
				cur += filler.Size();
			}
			// Copy the last partial string.
			std::memcpy(cur, filler.Data(), remCount);
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

String& String::AppendByte(u8 byte)
{
	if(m_Size + 1 >= GetAllocated())
		Reserve(GetAllocated() * 2 + 1);

	Data()[m_Size] = byte;
	Data()[m_Size + 1] = 0;

	++m_Size;
	return *this;
}

int String::Replace(StringView replace, StringView search, int first, int size)
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

int String::ReplaceRange(StringView replace, int first, int size)
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

	// Make room for the replace string.
	if(replace.Size() != size)
		std::memmove(data + first + replace.Size(), data + first + size, restSize + 1); // Including NUL

	// Copy the replace string.
	if(replace.Size() > 0)
		std::memcpy(data + first, replace.Data(), replace.Size());

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
	std::memmove(data + pos, data + pos + size, restSize + 1); // Including NUL

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
	auto end = ConstUTF8Iterator(Data() + Size());
	auto cur = begin;
	while(cur.Pointer() != end && IsSpace(*cur))
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

} // namespace core
} // namespace lux
