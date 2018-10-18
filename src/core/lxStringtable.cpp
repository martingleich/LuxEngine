#include "core/lxStringTable.h"
#include "core/lxUtil.h"
#include <unordered_map>
#include <cctype>

///////////////////////////////////////////////////////////////////////////////

namespace lux
{
namespace core
{

StringTableHandle::StringTableHandle() :
	m_Handle(nullptr)
{
}

StringTableHandle::StringTableHandle(const void* handle) :
	m_Handle(handle)
{
}

bool StringTableHandle::operator==(const StringTableHandle& other) const
{
	return (m_Handle == other.m_Handle);
}

bool StringTableHandle::operator!=(const StringTableHandle& other) const
{
	return !(*this == other);
}

const char* StringTableHandle::Data() const
{
	static const char null = '\0';
	if(!m_Handle)
		return &null;
	else
		return (reinterpret_cast<const char*>(m_Handle) + sizeof(int));
}

int StringTableHandle::GetHash() const
{
	return core::HashSequence((u8*)(&m_Handle), sizeof(void*));
}

int StringTableHandle::Size() const
{
	if(m_Handle)
		return *reinterpret_cast<const int*>(m_Handle);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct CheckEntry
{
	CheckEntry(const void* handle) :
		m_Handle(handle)
	{
	}

	const void* m_Handle;
	struct Hasher
	{
		int operator()(const CheckEntry& entry) const
		{
			return HashSequence(reinterpret_cast<const u8*>(entry.m_Handle) + sizeof(int),
				*reinterpret_cast<const int*>(entry.m_Handle));
		}
	};

	struct Equal
	{
		bool operator()(const CheckEntry& a, const CheckEntry& b) const
		{
			int aSize = *reinterpret_cast<const int*>(a.m_Handle) + sizeof(int);
			int bSize = *reinterpret_cast<const int*>(b.m_Handle) + sizeof(int);
			if(bSize != aSize)
				return false;
			return (memcmp(a.m_Handle, b.m_Handle, aSize) == 0);
		}
	};
};

struct StringTable::MemBlock
{
	MemBlock* next;
	int used;
	static const int DATA_SIZE = 4000;
	char data[DATA_SIZE];
};

struct StringTable::SelfType
{
	std::unordered_map<CheckEntry, StringTableHandle, CheckEntry::Hasher, CheckEntry::Equal> map;
	MemBlock* first;
	MemBlock* last;

	SelfType() :
		first(nullptr),
		last(nullptr)
	{
	}
};

StringTable&  StringTable::GlobalInstance()
{
	static StringTable* instance = nullptr;
	if(!instance)
		instance = LUX_NEW(StringTable);
	return *instance;
}

StringTable::StringTable() :
	self(LUX_NEW(SelfType))
{
}

StringTable::~StringTable()
{
	MemBlock* cur = self->first;
	while(cur) {
		MemBlock* next = cur->next;
		LUX_FREE(cur);
		cur = next;
	}

	LUX_FREE(self);
}

StringTableHandle StringTable::FindString(const StringView& str)
{
	return AddFindString(str, true);
}

StringTableHandle StringTable::AddString(const StringView& str)
{
	return AddFindString(str, false);
}

StringTableHandle StringTable::AddFindString(const StringView& str, bool find)
{
	int strSize = str.Size();
	int size = strSize + 1 + sizeof(int);

	MemBlock* block = GetMatchingPosition(size);

	void* handle = block->data + block->used;

	*reinterpret_cast<int*>(handle) = strSize;
	char* strPos = reinterpret_cast<char*>(handle) + sizeof(int);
	memcpy(strPos, str.Data(), strSize);
	strPos[strSize] = 0;

	CheckEntry checkEntry(handle);
	auto it = self->map.find(checkEntry);
	if(it != self->map.end()) {
		return it->second;
	} else {
		if(find)
			return StringTableHandle::INVALID;

		block->used += size;

		self->map.emplace(CheckEntry(handle), StringTableHandle(handle));
		return handle;
	}
}

StringTable::MemBlock* StringTable::GetMatchingPosition(int length)
{
	if(length > MemBlock::DATA_SIZE)
		throw GenericInvalidArgumentException("length", "String is to long for string table(max size=4000)");

	MemBlock* cur = self->first;
	while(cur) {
		if(MemBlock::DATA_SIZE - cur->used >= length)
			return cur;
		cur = cur->next;
	}

	cur = AddNewMemBlock();
	return cur;
}

StringTable::MemBlock* StringTable::AddNewMemBlock()
{
	MemBlock* newBlock = LUX_NEW(MemBlock);

	newBlock->next = nullptr;
	newBlock->used = 0;

	if(self->last)
		self->last->next = newBlock;
	self->last = newBlock;

	if(!self->first)
		self->first = self->last;

	return self->last;
}

///////////////////////////////////////////////////////////////////////////////

const StringTableHandle StringTableHandle::INVALID(nullptr);

}
}