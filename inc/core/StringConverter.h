#ifndef INCLUDED_LUX_STRINGCONVERTER_H
#define INCLUDED_LUX_STRINGCONVERTER_H
#include "DateAndTime.h"
#include "lxFormat.h"

namespace lux
{
namespace core
{

//! Converter to and from strings
namespace StringConverter
{
inline core::String IntToString(intmax_t num)
{
	char buffer[22]; // See format::IntToString for size reasoning
	format::IntToString(num, buffer);
	return buffer;
}

inline core::String UIntToString(intmax_t num)
{
	char buffer[22]; // See format::IntToString for size reasoning
	format::UIntToString(num, buffer);
	return buffer;
}

inline core::String& AppendIntToString(core::String& str, intmax_t num)
{
	char buffer[22]; // See format::IntToString for size reasoning
	format::IntToString(num, buffer);
	str.Append(buffer);
	return str;
}

inline core::String& AppendUIntToString(core::String& str, uintmax_t num)
{
	char buffer[22]; // See format::UIntToString for size reasoning
	format::UIntToString(num, buffer);
	str.Append(buffer);
	return str;
}

inline core::String& Append(core::String& str, unsigned long long num) { return AppendUIntToString(str, num); }
inline core::String& Append(core::String& str, unsigned long num) { return AppendUIntToString(str, num); }
inline core::String& Append(core::String& str, unsigned int num) { return AppendUIntToString(str, num); }
inline core::String& Append(core::String& str, unsigned short num) { return AppendUIntToString(str, num); }
inline core::String& Append(core::String& str, unsigned char num) { return AppendUIntToString(str, num); }

inline core::String& Append(core::String& str, long long num) { return AppendIntToString(str, num); }
inline core::String& Append(core::String& str, long num) { return AppendIntToString(str, num); }
inline core::String& Append(core::String& str, int num) { return AppendIntToString(str, num); }
inline core::String& Append(core::String& str, short num) { return AppendIntToString(str, num); }
inline core::String& Append(core::String& str, char num) { return AppendIntToString(str, num); }

template <typename... T>
inline core::String& AppendFormat(core::String& str, core::StringType format, T... args)
{
	core::StringSink sink(str);
	format::format(sink, format.data, args...);
	return str;
}

//! Convert a float to a string
inline core::String& Append(core::String& str, float value)
{
	char buffer[43]; // See format::FloatToString for size reasoning
	format::FloatToString(value, buffer);
	str.Append(buffer);
	return str;
}

//! Convert a float to a string
inline core::String& Append(core::String& str, double value)
{
	char buffer[43]; // See format::FloatToString for size reasoning
	format::FloatToString(value, buffer);
	str.Append(buffer);
	return str;
}

//! Convert a date to a string
inline core::String& Append(core::String& str, const core::DateAndTime& value)
{
	return AppendFormat(str, "~a", value);
}

//! Convert a string to a string.
inline core::String& Append(core::String& str, const core::String& value)
{
	str.Append(value);
	return str;
}

//! Convert a string to a string.
inline core::String& Append(core::String& str, const char* value)
{
	str.Append(value);
	return str;
}

template <typename T>
inline core::String ToString(const T& value)
{
	core::String str;
	return Append(str, value);
}

template <typename... T>
inline core::String Format(core::StringType format, T... args)
{
	core::String out;
	AppendFormat(out, format, args...);

	return std::move(out);
}

//! Create a float from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written if not null
\param [out] error Did an error occur, only written if not null
\return The parsed float
*/
inline float ParseFloat(const char* str, float errorValue = 0.0f, const char** nextChar = nullptr, bool* error = nullptr)
{
	int sign = 1;

	if(!*str) {
		if(nextChar)
			*nextChar = str;
		if(error)
			*error = true;
		return errorValue;
	}

	if(*str == '-') {
		sign = -1;
		++str;
	} else if(*str == '+') {
		sign = 1;
		++str;
	}

	if(strcmp(str, "inf") == 0) {
		if(error)
			*error = false;
		return sign * std::numeric_limits<float>::infinity();
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
	if(post != std::numeric_limits<unsigned int>::max())
		out += (float)post / numDigits;

	out *= sign;

	if(nextChar)
		*nextChar = str;

	if(error)
		*error = true;
	return out;
}

//! Create a integer from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written when not null
\return The parsed integer
*/
inline int ParseInt(const char* str, int errorValue = 0, const char** nextChar = nullptr, bool* error = nullptr)
{
	unsigned int value = 0;
	int numDigits = 0;
	int sign = 1;

	if(!*str) {
		if(nextChar)
			*nextChar = str;
		if(error)
			*error = true;
		return errorValue;
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
				if(error)
					*error = true;
				return errorValue;
			}
			++str;
		} else {
			break;
		}
	}

	if(nextChar)
		*nextChar = str;

	if(numDigits > 0) {
		if(error)
			*error = false;
		return (int)value*sign;
	} else {
		if(error)
			*error = true;
		return errorValue;
	}
}

//! Create a integer from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written if not null
\param [out] error Did an error occur, only written if not null
\return The parsed integer
*/
inline int ParseInt(const core::String& str, int errorValue = 0, const char** nextChar = nullptr, bool* error = nullptr)
{
	return ParseInt(str.Data(), errorValue, nextChar, error);
}

//! Create a float from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written if not null
\param [out] error Did an error occur, only written if not null
\return The parsed float
*/
inline float ParseFloat(const core::String& str, float errorValue = 0.0f, const char** nextChar = nullptr, bool* error = nullptr)
{
	return ParseFloat(str.Data(), errorValue, nextChar, error);
}

inline bool ParseBool(const core::String& str, bool errorValue = false, const char** nextChar = nullptr, bool* error = nullptr)
{
	LUX_UNUSED(str, nextChar, error);
	bool isTrue = str.StartsWith(str, "true", EStringCompare::CaseInsensitive);
	if(isTrue && nextChar)
		*nextChar = (str.First() + 4);
	bool isFalse = str.StartsWith(str, "false", EStringCompare::CaseInsensitive);
	if(isTrue && nextChar)
		*nextChar = (str.First() + 5);

	if(error)
		*error = !isTrue && !isFalse;
	if(isTrue)
		return true;
	if(isFalse)
		return false;
	return errorValue;
}

}
}
}

#endif // !INCLUDED_LUX_STRINGCONVERTER_H
