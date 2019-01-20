#ifndef INCLUDED_LUX_NAME_H
#define INCLUDED_LUX_NAME_H
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
	explicit Name(StringView str, int action = ADD, StringTable* table = nullptr);
	explicit Name(const char* str, int action = ADD, StringTable* table = nullptr) :
		Name(StringView(str, strlen(str)), action, table)
	{}

	void SetHandle(StringTableHandle handle);
	StringTableHandle GetHandle() const;
	Name& operator=(const Name& other);
	Name& operator=(StringView str);

	bool operator==(const Name& other) const;
	bool operator==(StringView other) const;
	bool operator!=(const Name& other) const;
	bool operator!=(StringView other) const;

	StringView AsView() const
	{
		return StringView(m_Handle.Data(), Size());
	}

	bool operator<(const Name& other) const;
	int Size() const;
	bool IsEmpty() const;

	void Set(StringView str, int action = ADD, StringTable* table = nullptr);

private:
	StringTableHandle m_Handle;
};

inline bool operator==(StringView str, const Name& namestring)
{
	return (namestring == str);
}

inline bool operator!=(StringView str, const Name& namestring)
{
	return (namestring != str);
}

template <>
struct HashType<Name>
{
	unsigned int operator()(const Name& n) const
	{
		return n.GetHandle().GetHash();
	}
};

inline void fmtPrint(format::Context& ctx, Name name, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	auto view = name.AsView();
	ctx.AddSlice(view.Size(), view.Data());
}

}
}

#endif // #ifndef INCLUDED_LUX_NAME_H
