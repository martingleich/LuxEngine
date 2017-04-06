#ifndef INCLUDED_STRINGCONVERTER_H
#define INCLUDED_STRINGCONVERTER_H
#include "lxString.h"
#include "DateAndTime.h"
#include "lxFormat.h"

namespace lux
{
namespace core
{

#if 0
//! Buffered string
/**
Use this to build long outputs from diffrent types
*/
class StringBuffer
{
private:
	string m_String;

public:
	//! Constructor
	/**
	\param reserved How many chars to reserve in the output strings
	*/
	BasicStringBuffer(u32 reserved = 32) :
		m_String(reserved)
	{
	}

	//! Append a list of value
	/**
	\param a Pointer to the list of value
	\param count The number of value
	\return selfreference
	*/
	template <typename T2>
	BasicStringBuffer& AppendList(const T2* list, size_t count)
	{
		Append('(');
		if(list && count > 0) {
			for(size_t i = 0; i < count - 1; ++i) {
				*this << list[i];
				Append(", ");
			}

			*this << list[count - 1];
		}
		Append(')');

		return *this;
	}

	//! Append a c-string
	/**
	\param s The string to append
	\param len The maximal number of chars to append
	\return selfreference
	*/
	BasicStringBuffer& Append(const char* s, u32 len)
	{
		for(u32 i = 0; i < len && *s; ++i, ++s)
			m_String.Append((T)*s);

		return *this;
	}

	//! Append a c-string
	/**
	\param s The string to append
	\return selfreference
	*/
	BasicStringBuffer& Append(const char* s)
	{
		for(; *s; ++s)
			m_String.Append((T)*s);

		return *this;
	}

	//! Append a c-string
	/**
	\param s The string to append
	\param len The maximal number of chars to append
	\return selfreference
	*/
	BasicStringBuffer& Append(const wchar_t* s, u32 len)
	{
		for(u32 i = 0; i < len && *s; ++i, ++s)
			m_String.Append((T)*s);

		return *this;
	}

	//! Append a c-string
	/**
	\param s The string to append
	\return selfreference
	*/
	BasicStringBuffer& Append(const wchar_t* s)
	{
		for(; *s; ++s)
			m_String.Append((T)*s);

		return *this;
	}

	//! Append a single char
	/**
	\param c The character to append
	\return selfreference
	*/
	BasicStringBuffer& Append(char c)
	{
		m_String.Append((T)c);
		return *this;
	}

	//! Append a single char
	/**
	\param c The character to append
	\return selfreference
	*/
	BasicStringBuffer& Append(wchar_t c)
	{
		m_String.Append((T)c);
		return *this;
	}

	//! The constructed string
	/**
	\return The string constructed by the buffer
	*/
	const basic_string<T>& GetString() const
	{
		return m_String;
	}

	//! Clear the stringbuffer
	void Clear()
	{
		m_String.Clear();
	}

	//! Append an int to the string buffer
	/**
	The output format is decimale.
	\param value The integer value to append
	\return selfreference
	*/
	BasicStringBuffer& AppendInt(int value)
	{
		int num = value;
		// Weniger abhänigkeiten und etwas schneller
		bool isNegative = false;
		if(num < 0) {
			num *= -1;
			isNegative = true;
		}

		// Puffer erstellen und null setzen
		char buffer[16] = {0};
		u32 end = 15;
		if(!num) {
			return (*this << '0');
		}

		do {
			--end;
			buffer[end] = (char)('0' + num % 10);
			num /= 10;
		} while(num && end);

		if(isNegative) {
			--end;
			buffer[end] = '-';
		}

		return this->Append(&buffer[end], 15 - end);
	}

	//! Append an unsigned int to the string buffer
	/**
	The output format is decimale.
	\param value The integer value to append
	\return selfreference
	*/
	BasicStringBuffer& AppendUint(unsigned int value)
	{
		// Puffer erstellen und null setzen
		unsigned int num = value;
		char buffer[16] = {0};
		u32 end = 15;
		if(!num) {
			return (*this << '0');
		}

		do {
			--end;
			buffer[end] = (char)('0' + num % 10);
			num /= 10;
		} while(num && end);

		return this->Append(&buffer[end], 15 - end);
	}

	//! Append a boolean to the string buffer
	/**
	If the boolean is set "true" is written, otherwise "false"
	\param value The boolean value to append
	\return selfreference
	*/
	BasicStringBuffer& AppendBool(bool value)
	{
		if(value)
			return this->Append("true", 4);
		else
			return this->Append("false", 5);
	}

	//! Append a date to the string buffer
	/**
	The outputformat is "day month dayID hour minute second year"
	Where day and month are three letter shorthands for the english day/month names
	All numbers are one based
	\param value The date value to append
	\return selfreference
	*/
	BasicStringBuffer& AppendDate(const core::DateAndTime& value)
	{
		static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

		char temp[25];
		sprintf_s(temp, 25, "%.3s %.3s %d %.2d:%.2d:%.2d %d",
			wday_name[(int)value.weekDay],
			mon_name[value.month - 1],
			value.dayOfMonth, value.hours,
			value.minutes, value.seconds,
			value.year);

		return this->Append(temp, 24);
	}

	//! Append a float to the string buffer
	/**
	The float is written with a dot seperator
	and with max presicion decimale places.
	\param value The float value to append
	\param precision The maximal number of decimale places
	\return selfreference
	*/
	BasicStringBuffer& AppendFloat(float value, int precision = 2)
	{
		char buffer[256];
		if(precision < 0)
			precision = 0;

		int i = sprintf_s(buffer, 256, "%.*f", precision, value);
		return this->Append(buffer, i);
	}

};

template <typename T>
class PrintIntAnyBase
{
	static_assert(std::is_integral<T>::value, "T must be an integral type");
public:
	PrintIntAnyBase(T integer, size_t base, size_t len = 0) :
		m_Int(integer),
		m_Base(base),
		m_Length(len)
	{
	}

	template <typename T2>
	BasicStringBuffer<T2>& operator()(BasicStringBuffer<T2>& e)
	{
		if(m_Base < 2)
			return e;

		T i = m_Int;
		if(i == 0) {
			e << '0';
			return e;
		}

		if(!std::is_unsigned<T2>::value) {
			if(i < 0)
				e << '-';
		}

		basic_string<T2> str;
		while(i > 0) {
			T r = i % m_Base;
			i = i / m_Base;

			str.Append(GetChar(r));
		}

		if(str.Size() < m_Length) {
			for(size_t i = str.Size(); i < m_Length; ++i)
				e << ' ';
		}

		for(size_t i = 0; i < str.Size(); ++i)
			e << str[str.Size() - i - 1];

		return e;
	}

	static char GetChar(T i)
	{
		if(i < 10)
			return '0' + i;
		else if(i < 10 + 26)
			return 'A' + i - 10;
		else
			return '?';
	}

private:
	T m_Int;
	size_t m_Base;
	size_t m_Length;
};

template <typename T>
PrintIntAnyBase<T> IntBase(T i, size_t base, size_t len)
{
	return PrintIntAnyBase<T>(i, base, len);
}

template <typename T>
PrintIntAnyBase<T> IntHex(T i)
{
	return PrintIntAnyBase<T>(i, 16, 0);
}

template <typename T>
PrintIntAnyBase<T> IntBin(T i)
{
	return PrintIntAnyBase<T>(i, 2, 0);
}

template <typename T, typename T2>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, PrintIntAnyBase<T2> ib)
{
	return ib(buffer);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, char value)
{
	return buffer.Append(value);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, wchar_t value)
{
	return buffer.Append(value);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, int value)
{
	return buffer.AppendInt(value);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, unsigned int value)
{
	return buffer.AppendUint(value);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, bool value)
{
	return buffer.AppendBool(value);
}

//! Stream oeprator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, const char* value)
{
	return buffer.Append(value);
}

//! Stream oeprator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, const wchar_t* value)
{
	return buffer.Append(value);
}

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, const core::DateAndTime& Date)
{
	return buffer.AppendDate(Date);
}

template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, const string value)
{
	return buffer.Append(value.c_str());
}

//! Stream oeprator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, const string value)
{
	return buffer.Append(value.c_str());
}

//! Stream oeprator

//! Stream operator
template <typename T>
BasicStringBuffer<T>& operator<<(BasicStringBuffer<T>& buffer, float value)
{
	return buffer.AppendFloat(value);
}

typedef BasicStringBuffer<char> StringBuffer;
typedef BasicStringBuffer<wchar_t> WStringBuffer;

#endif

//! Converter to and from strings
// TODO: Implement this functions via format library
class StringConverter
{
public:
	//! Convert an integer to a string
	static string ToString(size_t num)
	{
		char buffer[16] = {0};
		u32 end = 15;
		if(!num)
			return "0";

		do {
			--end;
			buffer[end] = (char)('0' + num % 10);
			num /= 10;
		} while(num && end);

		return string(buffer+end, 15-end);
	}

	//! Convert an integer to a string
	static string ToString(int num)
	{
		bool isNegative = false;
		if(num < 0) {
			num *= -1;
			isNegative = true;
		}

		char buffer[16] = {0};
		u32 end = 15;
		if(!num)
			return "0";

		do {
			--end;
			buffer[end] = (char)('0' + num % 10);
			num /= 10;
		} while(num && end);

		if(isNegative) {
			--end;
			buffer[end] = '-';
		}

		return string(buffer+end, 15-end);
	}

	//! Convert an float to a string
	static string ToString(float value)
	{
		char buffer[256];
		int precision = 2;
		if(precision < 0)
			precision = 0;

		int i = sprintf_s(buffer, 256, "%.*f", precision, value);
		return string(buffer, i);
	}

	//! Convert an date to a string
	static string ToString(const core::DateAndTime& value)
	{
		static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

		char temp[25];
		sprintf_s(temp, 25, "%.3s %.3s %d %.2d:%.2d:%.2d %d",
			wday_name[(int)value.weekDay],
			mon_name[value.month - 1],
			value.dayOfMonth, value.hours,
			value.minutes, value.seconds,
			value.year);

		return string(temp, 24);
	}

	template <typename... T>
	static string Format(string_type str, T... args)
	{
		string out;
		core::string_sink sink(out);
		format::format(sink, str.data, args...);

		return out;
	}
	
	//! Create a float from a string
	/**
	\param str The string to convert
	\param default The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed float
	*/
	static float ParseFloat(const string& str, float default = 0.0f, const char** nextChar = nullptr)
	{
		return ParseFloat(str.Data(), default, nextChar);
	}

	//! Create a float from a string
	/**
	\param str The string to convert
	\param default The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed float
	*/
	static float ParseFloat(const char* str, float default = 0.0f, const char** nextChar = nullptr)
	{
		int sign = 1;

		if(!*str) {
			if(nextChar)
				*nextChar = str;
			return default;
		}

		if(*str == '-') {
			sign = -1;
			++str;
		} else if(*str == '+') {
			sign = 1;
			++str;
		}

		unsigned int pre = 0;
		unsigned int post = std::numeric_limits<unsigned int>::max();

		unsigned int* value = &pre;
		int numDigits = 1;
		while(*str) {
			if(core::IsDigit(*str)) {
				unsigned int old = *value;
				*value *= 10;
				*value += *str - '0';
				numDigits *= 10;
				if(*value < old) {
					if(value == &pre) {
						value = &post;
						post = 0;
						numDigits = 1;
					} else if(value == &post)
						break;
				}
				++str;
			} else if(*str == '.') {
				if(value == &pre) {
					post = 0;
					value = &post;
					++str;
					numDigits = 1;
				} else if(value == &post)
					break;
			} else {
				break;
			}
		}

		float out = (float)pre;
		if(post != std::numeric_limits<unsigned int>::max()) {
			out += (float)post / numDigits;
		}

		out *= sign;

		if(nextChar)
			*nextChar = str;

		return out;
	}

	//! Create a integer from a string
	/**
	\param str The string to convert
	\param default The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed integer
	*/
	static int ParseInt(const string& str, int default = 0, const char** nextChar = nullptr)
	{
		return ParseInt(str.Data(), default, nextChar);
	}

	//! Create a integer from a string
	/**
	\param str The string to convert
	\param default The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed integer
	*/
	static int ParseInt(const char* str, int default = 0, const char** nextChar = nullptr)
	{
		unsigned int value = 0;
		int numDigits = 0;
		int sign = 1;

		if(!*str) {
			if(nextChar)
				*nextChar = str;
			return default;
		}

		if(*str == '-') {
			sign = -1;
			++str;
		} else if(*str == '+') {
			sign = 1;
			++str;
		}

		while(*str) {
			if(core::IsDigit(*str)) {
				value *= 10;
				value += *str - '0';
				numDigits++;
				if((sign == 1 && value > (unsigned int)std::numeric_limits<int>::max())
					|| (sign == -1 && value > (unsigned int)std::numeric_limits<int>::max())) {
					if(nextChar)
						*nextChar = str;
					return default;
				}
				++str;
			} else {
				break;
			}
		}

		if(nextChar)
			*nextChar = str;

		if(numDigits > 0)
			return (int)value*sign;
		else
			return default;
	}
};

}
}
#endif // !INCLUDED_STRINGCONVERTER_H
