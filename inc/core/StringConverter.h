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
	static core::String IntToString(intmax_t num)
	{
		char buffer[22]; // See format::IntToString for size reasoning
		format::IntToString(num, buffer);
		return buffer;
	}

	static core::String UIntToString(uintmax_t num)
	{
		char buffer[22]; // See format::UIntToString for size reasoning
		format::UIntToString(num, buffer);
		return buffer;
	}

public:
	static core::String ToString(unsigned long long num) { return UIntToString(num); }
	static core::String ToString(unsigned long num) { return UIntToString(num); }
	static core::String ToString(unsigned int num) { return UIntToString(num); }
	static core::String ToString(unsigned short num) { return UIntToString(num); }
	static core::String ToString(unsigned char num) { return UIntToString(num); }

	static core::String ToString(long long num) { return IntToString(num); }
	static core::String ToString(long num) { return IntToString(num); }
	static core::String ToString(int num) { return IntToString(num); }
	static core::String ToString(short num) { return IntToString(num); }
	static core::String ToString(char num) { return IntToString(num); }

	//! Convert a float to a string
	static core::String ToString(float value)
	{
		char buffer[43]; // See format::FloatToString for size reasoning
		format::FloatToString(value, buffer);
		return buffer;
	}

	//! Convert a float to a string
	static core::String ToString(double value)
	{
		char buffer[43]; // See format::FloatToString for size reasoning
		format::FloatToString(value, buffer);
		return buffer;
	}

	//! Convert a date to a string
	static core::String ToString(const core::DateAndTime& value)
	{
		return Format("~a", value);
	}

	//! Convert a string to a string.
	static const core::String& ToString(const core::String& value)
	{
		return value;
	}

	//! Convert a string to a string.
	static core::String ToString(const char* str)
	{
		return str;
	}

	template <typename... T>
	static core::String Format(core::StringType format, T... args)
	{
		core::String out;
		AppendFormat(out, format, args...);

		return std::move(out);
	}

	template <typename... T>
	static void AppendFormat(core::String& str, core::StringType format, T... args)
	{
		core::StringSink sink(str);
		format::format(sink, format.data, args...);
	}

	//! Create a float from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written if not null
	\param [out] error Did an error occur, only written if not null
	\return The parsed float
	*/
	static float ParseFloat(const core::String& str, float errorValue = 0.0f, const char** nextChar = nullptr, bool* error = nullptr)
	{
		return ParseFloat(str.Data(), errorValue, nextChar, error);
	}

	//! Create a float from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written if not null
	\param [out] error Did an error occur, only written if not null
	\return The parsed float
	*/
	static float ParseFloat(const char* str, float errorValue = 0.0f, const char** nextChar = nullptr, bool* error = nullptr)
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
	\param [out] nextChar The first character after the number, only written if not null
	\param [out] error Did an error occur, only written if not null
	\return The parsed integer
	*/
	static int ParseInt(const core::String& str, int errorValue = 0, const char** nextChar = nullptr, bool* error = nullptr)
	{
		return ParseInt(str.Data(), errorValue, nextChar, error);
	}

	//! Create a integer from a string
	/**
	\param str The string to convert
	\param errorValue The value which is returned if an error occurs
	\param [out] nextChar The first character after the number, only written when not null
	\return The parsed integer
	*/
	static int ParseInt(const char* str, int errorValue = 0, const char** nextChar = nullptr, bool* error = nullptr)
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
};

}
}
#endif // !INCLUDED_STRINGCONVERTER_H
