#include "core/lxStringTable.h"
#include "core/lxUtil.h"
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

StringTable&  StringTable::GlobalInstance()
{
	static StringTable* instance = nullptr;
	if(!instance)
		instance = LUX_NEW(StringTable);
	return *instance;
}

StringTable::StringTable() :
	m_First(nullptr),
	m_Last(nullptr)
{
}

StringTable::~StringTable()
{
	MemBlock* cur = m_First;
	while(cur) {
		MemBlock* next = cur->next;
		LUX_FREE(cur);
		cur = next;
	}
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
	auto itOpt = m_Map.Find(checkEntry);
	if(itOpt.HasValue()) {
		return itOpt.GetValue()->value;
	} else {
		if(find)
			return StringTableHandle::INVALID;

		block->used += size;

		m_Map.SetAndReplace(CheckEntry(handle), StringTableHandle(handle));
		return handle;
	}
}

StringTable::MemBlock* StringTable::GetMatchingPosition(int length)
{
	if(length > MemBlock::DATA_SIZE)
		throw GenericInvalidArgumentException("length", "String is to long for string table(max size=4000)");

	MemBlock* cur = m_First;
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

	if(m_Last)
		m_Last->next = newBlock;
	m_Last = newBlock;

	if(!m_First)
		m_First = m_Last;

	return m_Last;
}

///////////////////////////////////////////////////////////////////////////////

const StringTableHandle StringTableHandle::INVALID(nullptr);

}
}