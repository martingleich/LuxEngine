#ifndef INCLUDED_STRINGCONVERTER_H
#define INCLUDED_STRINGCONVERTER_H
#include "DateAndTime.h"
#include "lxFormat.h"

namespace lux
{
namespace core
{

//! Converter to and from strings
// TODO: Implement this functions via format library
class StringConverter
{
public:
	//! Convert a integer to a string
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

	//! Convert a integer to a string
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

	//! Convert a float to a string
	static string ToString(float value)
	{
		char buffer[256];
		int precision = 2;
		if(precision < 0)
			precision = 0;

		int i = sprintf(buffer, "%.*f", precision, value);
		return string(buffer, i);
	}

	//! Convert a date to a string
	static string ToString(const core::DateAndTime& value)
	{
		static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

		char temp[25];
		sprintf(temp, "%.3s %.3s %d %.2d:%.2d:%.2d %d",
			wday_name[(int)value.weekDay],
			mon_name[value.month - 1],
			value.dayOfMonth, value.hours,
			value.minutes, value.seconds,
			value.year);

		return string(temp, 24);
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
		core::string_sink sink(str);
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
