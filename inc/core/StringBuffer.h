#ifndef INCLUDED_LUX_STRING_BUFFER_H
#define INCLUDED_LUX_STRING_BUFFER_H
#include "core/StringConverter.h"

namespace lux
{
namespace core
{

//! Interface to build up strings.
/**
Has a more simple interface than using StringConverter directly.<br>
Is more effizient since overallocation is used.
*/
class StringBuffer
{
public:
	template <typename T>
	StringBuffer& operator<<(const T& value)
	{
		return Append(value);
	}

	template <typename T>
	StringBuffer& Append(const T& value)
	{
		StringConverter::Append(m_String, value);
		if(m_String.Size() == m_String.Allocated())
			m_String.Reserve(m_String.Allocated() * 2);
		return *this;
	}

	template <typename T>
	StringBuffer& AppendLine(const T& value)
	{
		StringConverter::Append(m_String, value);
		if(m_String.Size() == m_String.Allocated())
			m_String.Reserve(m_String.Allocated() * 2);
		m_String.Append('\n');
		return *this;
	}

	StringBuffer& AppendChar(u32 c)
	{
		m_String.Append(c);
		if(m_String.Size() == m_String.Allocated())
			m_String.Reserve(m_String.Allocated() * 2);
		return *this;
	}

	template <typename... Ts>
	StringBuffer& AppendFormat(core::StringType format, Ts... values)
	{
		StringConverter::AppendFormat(m_String, format, values...);
		if(m_String.Size() == m_String.Allocated())
			m_String.Reserve(m_String.Allocated() * 2);

		return *this;
	}

	template <typename... Ts>
	StringBuffer& AppendFormatLine(core::StringType format, Ts... values)
	{
		StringConverter::AppendFormat(m_String, format, values...);
		if(m_String.Size() == m_String.Allocated())
			m_String.Reserve(m_String.Allocated() * 2);
		m_String.Append('\n');

		return *this;
	}

	void Clear()
	{
		m_String.Clear();
	}

	const core::String& GetString() const
	{
		return m_String;
	}

	core::String TakeString()
	{
		core::String out(std::move(m_String));
		m_String.Clear();
		return std::move(out);
	}

private:
	core::String m_String;
};

}
}

#endif // #ifndef INCLUDED_LUX_STRING_BUFFER_H