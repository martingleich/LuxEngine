#ifndef INCLUDED_LX_NAME_H
#define INCLUDED_LX_NAME_H
#include "stringconverter.h"
#include "core/lxStringTable.h"
#include "lxUtil.h"
#include "lxFormat.h"
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

	void SetHandle(StringTableHandle handle);
	Name& operator=(const Name& other);
	Name& operator=(const char* str);
	Name& operator=(const string& str);

	const char* c_str() const;
	bool operator==(const Name& other) const;
	bool operator==(const char* str) const;
	bool operator==(const string& str) const;
	bool operator!=(const Name& other) const;
	bool operator!=(const char* str) const;
	bool operator!=(const string& str) const;

	operator bool() const
	{
		return (Size() != 0);
	}

	bool operator<(const Name& other) const;
	size_t Size() const;
	bool IsEmpty() const;

	void Set(const char* str, int action = ADD, StringTable* table = nullptr);
	void Set(const string& str, int action = ADD, StringTable* table = nullptr);

private:
	StringTableHandle m_Handle;
};

inline bool operator==(const char* cstr, const Name& namestring)
{
	return (namestring == cstr);
}

inline bool operator!=(const char* cstr, const Name& namestring)
{
	return (namestring != cstr);
}

inline bool operator==(const string& str, const Name& namestring)
{
	return (namestring == str);
}

inline bool operator!=(const string& str, const Name& namestring)
{
	return (namestring != str);
}

template <>
struct HashType<Name>
{
	size_t operator()(const Name& n) const
	{
		if(n.Size() == 0)
			return 0;

		return HashSequence(reinterpret_cast<const u8*>(n.c_str()), n.Size());
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