#ifndef INCLUDED_LUX_STRINGCONVERTER_H
#define INCLUDED_LUX_STRINGCONVERTER_H
#include "core/Clock.h"
#include "core/lxFormat.h"

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
inline core::String& AppendFormat(core::String& str, core::StringView format, T... args)
{
	core::StringSink sink(str);
	format::format(sink, format.Data(), args...);
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
inline core::String Format(core::StringView format, T... args)
{
	core::String out;
	AppendFormat(out, format, args...);

	return std::move(out);
}

enum class EParseError
{
	OK=0,
	Error=1,
	Overflow=2,
	EmptyInput=3,
};
//! Create a float from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written if not null
\param [out] error Did an error occur, only written if not null
\return The parsed float
*/
inline float ParseFloat(StringView str, float errorValue = 0.0f, int* nextChar = nullptr, EParseError* error = nullptr)
{
	int sign = 1;

	if(str.IsEmpty()) {
		if(nextChar)
			*nextChar = 0;
		if(error)
			*error = EParseError::EmptyInput;
		return errorValue;
	}

	const char* p = str.Data();
	const char* end = str.Data() + str.Size();
	if(*p == '-') {
		sign = -1;
		++p;
	} else if(*p == '+') {
		sign = 1;
		++p;
	}

	if(end - p >= 3 && memcmp(p, "inf", 3) == 0) {
		if(error)
			*error = EParseError::Error;
		return sign * std::numeric_limits<float>::infinity();
	}

	unsigned int pre = 0;
	unsigned int post = std::numeric_limits<unsigned int>::max();

	unsigned int* value = &pre;
	int numDigits = 1;
	while(p != end) {
		if(*p >= '0' && *p <= '9') {
			unsigned int old = *value;
			unsigned int dvalue = *p - '0';
			*value *= 10;
			*value += dvalue;
			numDigits *= 10;
			if(*value < old) {
				if(value == &pre) {
					value = &post;
					post = 0;
					numDigits = 1;
				} else if(value == &post)
					break;
			}
			++p;
		} else if(*p == '.') {
			if(value == &pre) {
				post = 0;
				value = &post;
				++p;
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
		*nextChar = p - str.Data();

	if(error)
		*error = EParseError::Error;
	return out;
}

//! Create a integer from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written when not null
\return The parsed integer
*/
inline int ParseInt(StringView str, int errorValue = 0, int* nextChar = nullptr, EParseError* error = nullptr)
{
	unsigned int value = 0;
	int numDigits = 0;
	int sign = 1;

	if(str.IsEmpty()) {
		if(nextChar)
			*nextChar = 0;
		if(error)
			*error = EParseError::EmptyInput;
		return errorValue;
	}

	const char* p = str.Data();
	const char* end = str.Data() + str.Size();
	if(*p == '-') {
		sign = -1;
		++p;
	} else if(*p == '+') {
		sign = 1;
		++p;
	}

	unsigned int maxValue = sign==1 ? (unsigned int)std::numeric_limits<int>::max() : ((unsigned int)std::numeric_limits<int>::max()+1);
	while(p != end) {
		if(*p >= '0' && *p <= '9') {
			unsigned int dvalue = *p - '0';
			if(value > (maxValue-dvalue)/10) { 
				if(nextChar)
					*nextChar = p - str.Data();
				if(error)
					*error = EParseError::Overflow;
				return errorValue;
			}
			value = 10*value + dvalue;
			numDigits++;
			++p;
		} else {
			break;
		}
	}

	if(nextChar)
		*nextChar = p - str.Data();

	if(numDigits > 0) {
		if(error)
			*error = EParseError::OK;
		return (int)value*sign;
	} else {
		if(error)
			*error = EParseError::Error;
		return errorValue;
	}
}

inline bool ParseBool(core::StringView str, bool errorValue = false, int* nextChar = nullptr, EParseError* error = nullptr)
{
	LUX_UNUSED(str, nextChar, error);
	bool isTrue = str.StartsWith("true", EStringCompare::CaseInsensitive);
	if(isTrue && nextChar)
		*nextChar = 4;
	bool isFalse = str.StartsWith("false", EStringCompare::CaseInsensitive);
	if(isTrue && nextChar)
		*nextChar = 5;

	if(error)
		*error = !isTrue && !isFalse ? EParseError::Error : EParseError::OK;
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
