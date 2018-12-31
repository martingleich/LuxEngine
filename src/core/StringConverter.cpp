#include "core/StringConverter.h"

namespace lux
{
namespace core
{
namespace StringConverter
{

String IntToString(intmax_t num)
{
	String out;
	format::format(out, "{}", num);
	return out;
}

String UIntToString(intmax_t num)
{
	String out;
	format::format(out, "{}", num);
	return out;
}

String& AppendIntToString(String& str, intmax_t value)
{
	return AppendFormat(str, "{}", value);
}

String& AppendUIntToString(String& str, uintmax_t value)
{
	return AppendFormat(str, "{}", value);
}

String& Append(String& str, float value)
{
	return AppendFormat(str, "{}", value);
}

String& Append(String& str, double value)
{
	return AppendFormat(str, "{}", value);
}

String& Append(String& str, const DateAndTime& value)
{
	return AppendFormat(str, "{}", value);
}

String& Append(String& str, const StringView& value)
{
	return str.Append(value);
}

float ParseFloat(StringView str, float errorValue, int* nextChar, EParseError* error)
{
	int sign = 1;

	if(str.IsEmpty()) {
		if(nextChar)
			*nextChar = 0;
		if(error)
			*error = EParseError::EmptyInput;
		return errorValue;
	}

	int i = 0;
	if(str[i] == '-') {
		sign = -1;
		++i;
	} else if(str[i] == '+') {
		sign = 1;
		++i;
	}

	if(i + 3 <= str.Size() && (str[i] == 'i' && str[i+1] == 'n' && str[i+2]=='f')) {
		i += 3;
		if(nextChar)
			*nextChar = i;
		if(error)
			*error = EParseError::Error;
		return sign * std::numeric_limits<float>::infinity();
	}

	unsigned int pre = 0;
	unsigned int post = std::numeric_limits<unsigned int>::max();

	unsigned int* value = &pre;
	int numDigits = 1;
	while(i < str.Size()) {
		if(str[i] >= '0' && str[i] <= '9') {
			unsigned int old = *value;
			unsigned int dvalue = str[i] - '0';
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
			++i;
		} else if(str[i] == '.') {
			if(value == &pre) {
				post = 0;
				value = &post;
				++i;
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
		*nextChar = i;

	if(error)
		*error = EParseError::Error;
	return out;
}

int ParseInt(StringView str, int errorValue, int* nextChar, EParseError* error)
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

	int i = 0;
	if(str[i] == '-') {
		sign = -1;
		++i;
	} else if(str[i] == '+') {
		sign = 1;
		++i;
	}

	unsigned int maxValue = sign==1 ? (unsigned int)std::numeric_limits<int>::max() : ((unsigned int)std::numeric_limits<int>::max()+1);
	while(i < str.Size()) {
		if(str[i] >= '0' && str[i] <= '9') {
			unsigned int dvalue = str[i] - '0';
			if(value > (maxValue-dvalue)/10) { 
				if(nextChar)
					*nextChar = i;
				if(error)
					*error = EParseError::Overflow;
				return errorValue;
			}
			value = 10*value + dvalue;
			numDigits++;
			++i;
		} else {
			break;
		}
	}

	if(nextChar)
		*nextChar = i;

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

bool ParseBool(StringView str, bool errorValue, int* nextChar, EParseError* error)
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
