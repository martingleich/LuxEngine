#include "core/lxName.h"

namespace lux
{
namespace core
{

static int strcicmp(char const *a, char const *b)
{
	while(ToLowerChar(*a) == ToLowerChar(*b)) {
		++b;
		++a;
		if(*a == '\0')
			return 0;
	}

	return (ToLowerChar(*a) - ToLowerChar(*b));
}

Name::Name() :
	m_Handle(StringTableHandle::INVALID)
{
}

Name::Name(const StringType& str, int action, StringTable* table) :
	m_Handle(StringTableHandle::INVALID)
{
	if(str.data)
		Set(str, action, table);
}

void Name::SetHandle(StringTableHandle handle)
{
	m_Handle = handle;
}

Name& Name::operator=(const Name& other)
{
	SetHandle(other.m_Handle);
	return *this;
}

void Name::Set(const StringType& str, int action, StringTable* table)
{
	lxAssert("Assign null to name string." && str.data);

	if(!table)
		table = &StringTable::GlobalInstance();

	if(action == FIND_ONLY)
		SetHandle(table->FindString(str));
	else if(action == ADD)
		SetHandle(table->AddString(str));
	else
		(void)0;
}

Name& Name::operator=(const StringType& str)
{
	Set(str.data, ADD, nullptr);
	return *this;
}

const char* Name::c_str() const
{
	return m_Handle.c_str();
}

bool Name::operator==(const StringType& other) const
{
	lxAssert("Compare null to name string." && other.data);
	if(!other.data)
		return false;

	return (strcicmp(c_str(), other.data) == 0);
}

bool Name::operator==(const Name& other) const
{
	return (m_Handle == other.m_Handle);
}

bool Name::operator!=(const Name& other) const
{
	return !(*this == other);
}

bool Name::operator!=(const StringType& str) const
{
	return !(*this == str);
}

bool Name::operator<(const Name& other) const
{
	// Simple pointer compare is ok since, the pointer values for each execution are the same
	return c_str() < other.c_str();
}

size_t Name::Size() const
{
	return m_Handle.Size();
}

bool Name::IsEmpty() const
{
	return (Size() == 0);
}

///////////////////////////////////////////////////////////////////////////////

const Name Name::INVALID(nullptr);

}
}
