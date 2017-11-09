#ifndef INCLUDED_STRING_BUFFER_H
#define INCLUDED_STRING_BUFFER_H
#include "core/StringConverter.h"

namespace lux
{
namespace core
{

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
		m_String.Append(StringConverter::ToString(value));
		return *this;
	}

	StringBuffer& AppendChar(u32 c)
	{
		m_String.Append(c);
		return *this;
	}

	template <typename... Ts>
	StringBuffer& AppendFormat(core::StringType format, Ts... values)
	{
		StringConverter::AppendFormat(m_String, format, values...);
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

private:
	core::String m_String;
};

}
}

#endif // #ifndef INCLUDED_STRING_BUFFER_H