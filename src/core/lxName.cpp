#include "core/lxName.h"

namespace lux
{
namespace core
{

Name::Name() :
	m_Handle(StringTableHandle::INVALID)
{
}

Name::Name(const char* str, int action, StringTable* table) :
	m_Handle(StringTableHandle::INVALID)
{
	if(str)
		Set(str, action, table);
}
Name::Name(const String& str, int action, StringTable* table) :
	m_Handle(StringTableHandle::INVALID)
{
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

void Name::Set(const char* str, int action, StringTable* table)
{
	lxAssert("Assign null to name string." && str);

	if(!table)
		table = &StringTable::GlobalInstance();

	if(action == FIND_ONLY)
		SetHandle(table->FindString(str));
	else if(action == ADD)
		SetHandle(table->AddString(str));
	else
		(void)0;
}

void Name::Set(const String& str, int action, StringTable* table)
{
	Set(str.Data(), action, table);
}

Name& Name::operator=(const char* str)
{
	Set(str, ADD, nullptr);
	return *this;
}
Name& Name::operator=(const String& str)
{
	Set(str, ADD, nullptr);
	return *this;
}

const char* Name::c_str() const
{
	return m_Handle.c_str();
}

int Name::GetHash() const
{
	return m_Handle.GetHash();
}

bool Name::operator==(const char* other) const
{
	if(!other)
		return false;

	return (strcmp(c_str(), other) == 0);
}

bool Name::operator==(const String& other) const
{
	return (strcmp(c_str(), other.Data()) == 0);
}

bool Name::operator==(const Name& other) const
{
	return (m_Handle == other.m_Handle);
}

bool Name::operator!=(const Name& other) const
{
	return !(*this == other);
}

bool Name::operator!=(const char* str) const
{
	return !(*this == str);
}

bool Name::operator!=(const String& str) const
{
	return !(*this == str);
}

bool Name::operator<(const Name& other) const
{
	// Simple pointer compare is ok since, the pointer values for each execution are the same
	return c_str() < other.c_str();
}

int Name::Size() const
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
