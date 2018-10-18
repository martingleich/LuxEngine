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
LUX_API String IntToString(intmax_t num);
LUX_API String UIntToString(intmax_t num);

LUX_API String& AppendIntToString(String& str, intmax_t num);
LUX_API String& AppendUIntToString(String& str, uintmax_t num);

inline String& Append(String& str, unsigned long long num) { return AppendUIntToString(str, num); }
inline String& Append(String& str, unsigned long num) { return AppendUIntToString(str, num); }
inline String& Append(String& str, unsigned int num) { return AppendUIntToString(str, num); }
inline String& Append(String& str, unsigned short num) { return AppendUIntToString(str, num); }
inline String& Append(String& str, unsigned char num) { return AppendUIntToString(str, num); }

inline String& Append(String& str, long long num) { return AppendIntToString(str, num); }
inline String& Append(String& str, long num) { return AppendIntToString(str, num); }
inline String& Append(String& str, int num) { return AppendIntToString(str, num); }
inline String& Append(String& str, short num) { return AppendIntToString(str, num); }
inline String& Append(String& str, char num) { return AppendIntToString(str, num); }

//! Convert a float to a string
LUX_API String& Append(String& str, float value);
//! Convert a float to a string
LUX_API String& Append(String& str, double value);
//! Convert a date to a string
LUX_API String& Append(String& str, const DateAndTime& value);
//! Convert a string to a string.
LUX_API String& Append(String& str, const StringView& value);

template <typename... T>
inline String& AppendFormat(String& str, StringView format, T... args)
{
	StringSink sink(str);
	format::format(sink, format::Slice(format.Size(), format.Data()), args...);
	return str;
}

template <typename T>
inline String ToString(const T& value)
{
	String str;
	return Append(str, value);
}

template <typename... T>
inline String Format(StringView format, T... args)
{
	String out;
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
LUX_API float ParseFloat(StringView str, float errorValue = 0.0f, int* nextChar = nullptr, EParseError* error = nullptr);

//! Create a integer from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written when not null
\param [out] error Did an error occur, only written if not null
\return The parsed integer
*/
LUX_API int ParseInt(StringView str, int errorValue = 0, int* nextChar = nullptr, EParseError* error = nullptr);

//! Create a boolean from a string
/**
\param str The string to convert
\param errorValue The value which is returned if an error occurs
\param [out] nextChar The first character after the number, only written when not null
\param [out] error Did an error occur, only written if not null
\return The parsed integer
*/
LUX_API bool ParseBool(StringView str, bool errorValue = false, int* nextChar = nullptr, EParseError* error = nullptr);

}
}
}

#endif // !INCLUDED_LUX_STRINGCONVERTER_H
