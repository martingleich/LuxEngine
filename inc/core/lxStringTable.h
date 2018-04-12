#ifndef INCLUDED_LUX_STRING_TABLE_H
#define INCLUDED_LUX_STRING_TABLE_H
#include "core/lxMemory.h"
#include "core/lxString.h"

namespace lux
{
namespace core
{

//! A handle a entry in the string table
class LUX_API StringTableHandle
{
public:
	//! The invalid table handle
	static const StringTableHandle INVALID;

public:
	StringTableHandle();
	StringTableHandle(const void* handle);
	bool operator==(const StringTableHandle& other) const;
	bool operator!=(const StringTableHandle& other) const;
	const char* c_str() const;
	int Size() const;
	int GetHash() const;

private:
	const void* m_Handle;
};

//! The string table for names
class LUX_API StringTable
{
public:
	static StringTable& GlobalInstance();

public:
	StringTable();
	~StringTable();

	//! Check if a string exist in the string table
	/**
	Returns invalid handle if the string does not exist.
	\param str The string to check
	\return The handle of the entry
	*/
	StringTableHandle FindString(const StringType& str);

	//! Add a string to the string table
	/**
	If the string is already in the table, don't add it again.
	\param str The string to add
	\return The handle to the entry
	*/
	StringTableHandle AddString(const StringType& str);

private:
	struct MemBlock;
	MemBlock* GetMatchingPosition(int length);
	MemBlock* AddNewMemBlock();

	StringTableHandle AddFindString(const char* str, bool find);

private:
	struct SelfType;
	SelfType* self;
};

}
}

#endif // #ifndef INCLUDED_LUX_STRING_TABLE_H
