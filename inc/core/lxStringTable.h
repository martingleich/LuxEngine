#ifndef INCLUDED_LUX_STRING_TABLE_H
#define INCLUDED_LUX_STRING_TABLE_H
#include "core/lxMemory.h"
#include "core/lxString.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace core
{

//! A handle a entry in the string table
class StringTableHandle
{
public:
	//! The invalid table handle
	LUX_API static const StringTableHandle INVALID;

public:
	LUX_API StringTableHandle();
	LUX_API StringTableHandle(const void* handle);
	LUX_API bool operator==(const StringTableHandle& other) const;
	LUX_API bool operator!=(const StringTableHandle& other) const;
	LUX_API const char* Data() const;
	LUX_API int Size() const;
	LUX_API int GetHash() const;

private:
	const void* m_Handle;
};

//! The string table for names
class StringTable
{
public:
	LUX_API static StringTable& GlobalInstance();

public:
	LUX_API StringTable();
	LUX_API ~StringTable();

	//! Check if a string exist in the string table
	/**
	Returns invalid handle if the string does not exist.
	\param str The string to check
	\return The handle of the entry
	*/
	LUX_API StringTableHandle FindString(const StringView& str);

	//! Add a string to the string table
	/**
	If the string is already in the table, don't add it again.
	\param str The string to add
	\return The handle to the entry
	*/
	LUX_API StringTableHandle AddString(const StringView& str);

private:
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

		struct Compare
		{
			bool Equal(const CheckEntry& a, const CheckEntry& b) const
			{
				int aSize = *reinterpret_cast<const int*>(a.m_Handle) + sizeof(int);
				int bSize = *reinterpret_cast<const int*>(b.m_Handle) + sizeof(int);
				if(bSize != aSize)
					return false;
				return (std::memcmp(a.m_Handle, b.m_Handle, aSize) == 0);
			}
		};
	};

	struct MemBlock
	{
		MemBlock* next;
		int used;
		static const int DATA_SIZE = 4000;
		char data[DATA_SIZE];
	};
	MemBlock* GetMatchingPosition(int length);
	MemBlock* AddNewMemBlock();

	StringTableHandle AddFindString(const StringView& str, bool find);

private:
	HashMap<CheckEntry, StringTableHandle, CheckEntry::Hasher, CheckEntry::Compare> m_Map;
	MemBlock* m_First;
	MemBlock* m_Last;
};

}
}

#endif // #ifndef INCLUDED_LUX_STRING_TABLE_H
