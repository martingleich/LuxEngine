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

const char* StringTableHandle::c_str() const
{
	static const char null = '\0';
	if(!m_Handle)
		return &null;
	else
		return (reinterpret_cast<const char*>(m_Handle) + sizeof(size_t));
}

size_t StringTableHandle::GetHash() const
{
	return reinterpret_cast<size_t>(m_Handle);
}

size_t StringTableHandle::Size() const
{
	if(m_Handle)
		return *reinterpret_cast<const size_t*>(m_Handle);
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
		size_t operator()(const CheckEntry& entry) const
		{
			return HashSequence(reinterpret_cast<const u8*>(entry.m_Handle) + sizeof(size_t),
				*reinterpret_cast<const size_t*>(entry.m_Handle));
		}
	};

	struct Equal
	{
		bool operator()(const CheckEntry& a, const CheckEntry& b) const
		{
			size_t aSize = *reinterpret_cast<const size_t*>(a.m_Handle) + sizeof(size_t);
			size_t bSize = *reinterpret_cast<const size_t*>(b.m_Handle) + sizeof(size_t);
			if(bSize != aSize)
				return false;
			return (memcmp(a.m_Handle, b.m_Handle, aSize) == 0);
		}
	};
};

struct StringTable::MemBlock
{
	MemBlock* next;
	size_t used;
	static const size_t DATA_SIZE = 4000;
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

StringTableHandle StringTable::FindString(const StringType& str)
{
	return AddFindString(str.data, true);
}

StringTableHandle StringTable::AddString(const StringType& str)
{
	return AddFindString(str.data, false);
}

StringTableHandle StringTable::AddFindString(const char* str, bool find)
{
	if(!str)
		throw InvalidArgumentException("str", "Entry in stringtable may not be empty.");

	size_t strSize = strlen(str);
	size_t size = strSize + 1 + sizeof(size_t);

	MemBlock* block = GetMatchingPosition(size);

	void* handle = block->data + block->used;

	*reinterpret_cast<size_t*>(handle) = strSize;
	char* strPos = reinterpret_cast<char*>(handle) + sizeof(size_t);

	/*
	ifconst(LUX_STRING_TABLE_CASE_HANDLING == 0) {
		for(size_t i = 0; i < strSize + 1; ++i)
			strPos[i] = static_cast<char>(std::tolower(str[i]));
	} else {
	*/
		memcpy(strPos, str, strSize + 1);
	//}

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

StringTable::MemBlock* StringTable::GetMatchingPosition(size_t length)
{
	if(length > MemBlock::DATA_SIZE)
		throw Exception("String is to long for string table");

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