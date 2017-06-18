#ifndef INCLUDED_STRINGCONVERTER_H
#define INCLUDED_STRINGCONVERTER_H
#include "DateAndTime.h"
#include "lxFormat.h"

namespace lux
{
namespace core
{

//! Converter to and from strings
class StringConverter
{
private:
	static string IntToString(intmax_t num)
	{
		char buffer[22]; // See format::IntToString for size reasoning
		format::IntToString(num, buffer);
		return buffer;
	}

	static string UIntToString(uintmax_t num)
	{
		char buffer[22]; // See format::UIntToString for size reasoning
		format::UIntToString(num, buffer);
		return buffer;
	}
public:
	static string ToString(unsigned long long num) { return UIntToString(num); }
	static string ToString(unsigned long num) { return UIntToString(num); }
	static string ToString(unsigned int num) { return UIntToString(num); }
	static string ToString(unsigned short num) { return UIntToString(num); }
	static string ToString(unsigned char num) { return UIntToString(num); }

	static string ToString(long long num) { return IntToString(num); }
	static string ToString(long num) { return IntToString(num); }
	static string ToString(int num) { return IntToString(num); }
	static string ToString(short num) { return IntToString(num); }
	static string ToString(char num) { return IntToString(num); }

	//! Convert a float to a string
	static string ToString(float value)
	{
		char buffer[43]; // See format::FloatToString for size reasoning
		format::FloatToString(value, buffer);
		return buffer;
	}

	//! Convert a float to a string
	static string ToString(double value)
	{
		char buffer[43]; // See format::FloatToString for size reasoning
		format::FloatToString(value, buffer);
		return buffer;
	}

	//! Convert a date to a string
	static string ToString(const core::DateAndTime& value)
	{
		return Format("~a", value);
	}

	//! Convert a string to a string.
	static const string& ToString(const string& value)
	{
		return value;
	}

	//! Convert a string to a string.
	static string ToString(const char* str)
	{
		return str;
	}

	template <typename... T>
	static string Format(string_type format, T... args)
	{
		string out;
		AppendFormat(out, format, args...);

		return std::move(out);
	}
	
	template <typename... T>
	static void AppendFormat(string& str, string_type format, T... args)
	{
		core::StringSink sink(str);
		format::format(sink, format.data, args...);
	}

	//! Create a float from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed float
	*/
	static float ParseFloat(const string& str, float errorValue = 0.0f, const char** nextChar = nullptr)
	{
		return ParseFloat(str.Data(), errorValue, nextChar);
	}

	//! Create a float from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed float
	*/
	static float ParseFloat(const char* str, float errorValue = 0.0f, const char** nextChar = nullptr)
	{
		int sign = 1;

		if(!*str) {
			if(nextChar)
				*nextChar = str;
			return errorValue;
		}

		if(*str == '-') {
			sign = -1;
			++str;
		} else if(*str == '+') {
			sign = 1;
			++str;
		}

		if(strcmp(str, "inf") == 0)
			return sign * std::numeric_limits<float>::infinity();

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
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed integer
	*/
	static int ParseInt(const string& str, int errorValue = 0, const char** nextChar = nullptr)
	{
		return ParseInt(str.Data(), errorValue, nextChar);
	}

	//! Create a integer from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed integer
	*/
	static int ParseInt(const char* str, int errorValue = 0, const char** nextChar = nullptr)
	{
		unsigned int value = 0;
		int numDigits = 0;
		int sign = 1;

		if(!*str) {
			if(nextChar)
				*nextChar = str;
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
					return errorValue;
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
			return errorValue;
	}
};

}
}
#endif // !INCLUDED_STRINGCONVERTER_H
