#ifndef INCLUDED_LX_NAME_H
#define INCLUDED_LX_NAME_H
#include "core/StringConverter.h"
#include "core/lxStringTable.h"
#include "core/lxUtil.h"
#include "core/lxFormat.h"

namespace lux
{
namespace core
{

//! A string representing a name
class LUX_API Name
{
public:
	//! The invalid/empty name string
	static const Name INVALID;

	static const int FIND_ONLY = 1;
	static const int ADD = 0;
public:
	//! Construct an empty name string
	Name();

	//! Construct from name
	/**
	\param str The name
	\param findOnly If the name isn't already in the table and this is true, the string isn't put in the table
		and a empty name is returned, if it's false, the string is added into the table
	*/
	Name(const char* str, int action = ADD, StringTable* table = nullptr);
	Name(const String& str, int action = ADD, StringTable* table = nullptr);

	void SetHandle(StringTableHandle handle);
	Name& operator=(const Name& other);
	Name& operator=(const char* str);
	Name& operator=(const String& str);

	const char* c_str() const;
	size_t GetHash() const;
	bool operator==(const Name& other) const;
	bool operator==(const char* str) const;
	bool operator==(const String& str) const;
	bool operator!=(const Name& other) const;
	bool operator!=(const char* str) const;
	bool operator!=(const String& str) const;

	operator bool() const
	{
		return (Size() != 0);
	}

	operator StringType() const
	{
		return StringType(c_str());
	}

	bool operator<(const Name& other) const;
	size_t Size() const;
	bool IsEmpty() const;

	void Set(const char* str, int action = ADD, StringTable* table = nullptr);
	void Set(const String& str, int action = ADD, StringTable* table = nullptr);

private:
	StringTableHandle m_Handle;
};

inline bool operator==(const char* cstr, const Name& namestring)
{
	return (namestring == cstr);
}

inline bool operator==(const String& str, const Name& namestring)
{
	return (namestring == str);
}

inline bool operator!=(const char* cstr, const Name& namestring)
{
	return (namestring != cstr);
}

inline bool operator!=(const String& cstr, const Name& namestring)
{
	return (namestring != cstr);
}

template <>
struct HashType<Name>
{
	size_t operator()(const Name& n) const
	{
		return n.GetHash();
	}
};

inline void conv_data(format::Context& ctx, Name name, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	format::ConvertAddString(ctx, format::StringType::Unicode, name.c_str(), name.Size());
}

}
}

#endif // #ifndef INCLUDED_LX_NAME_H
