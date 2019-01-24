#include "core/lxName.h"

namespace lux
{
namespace core
{

const Name Name::INVALID;

Name::Name() :
	m_Handle(StringTableHandle::INVALID)
{
}

Name::~Name()
{
}

Name::Name(StringView str, int action, StringTable* table) :
	m_Handle(StringTableHandle::INVALID)
{
	Set(str, action, table);
}

void Name::SetHandle(StringTableHandle handle)
{
	m_Handle = handle;
}

StringTableHandle Name::GetHandle() const
{
	return m_Handle;
}

Name& Name::operator=(const Name& other)
{
	SetHandle(other.m_Handle);
	return *this;
}

void Name::Set(StringView str, int action, StringTable* table)
{
	if(!table)
		table = &StringTable::GlobalInstance();

	if(action == FIND_ONLY)
		SetHandle(table->FindString(str));
	else if(action == ADD)
		SetHandle(table->AddString(str));
	else
		(void)0;
}

Name& Name::operator=(StringView str)
{
	Set(str, ADD, nullptr);
	return *this;
}
bool Name::operator==(StringView other) const
{
	return other.Equal(StringView(m_Handle.Data(), m_Handle.Size()));
}

bool Name::operator==(const Name& other) const
{
	return (m_Handle == other.m_Handle);
}

bool Name::operator!=(StringView other) const
{
	return !other.Equal(StringView(m_Handle.Data(), m_Handle.Size()));
}

bool Name::operator!=(const Name& other) const
{
	return !(*this == other);
}


bool Name::operator<(const Name& other) const
{
	// Simple pointer compare is ok since, the pointer values for each execution are the same
	return m_Handle.Data() < other.m_Handle.Data();
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


}
}
