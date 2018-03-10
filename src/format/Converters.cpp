#include "format/Converters.h"
#include "format/ConvInternal.h"
#include "format/UnicodeConversion.h"

namespace format
{

void fmtPrint(Context& ctx, const char* data, Placeholder& placeholder)
{
	if(placeholder.type == 'a' || placeholder.type == 's') {
		if(data)
			ctx.AddTerminatedSlice(data, true);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for char string pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

template <typename T>
void write_integer(Context& ctx, int base, bool sign, T value, Placeholder& placeholder)
{
	auto& facet = ctx.GetLocale()->GetNumericalFacet();

	if(sign || placeholder.plus)
		ctx.AddSlice(sign ? facet.Minus : facet.Plus);

	char buffer[64];
	char* cur = buffer;

	if(placeholder.hash) {
		if(base == 2) {
			*cur++ = 'b';
		} else if(base == 8) {
			*cur++ = 'o';
		} else if(base == 16) {
			*cur++ = 'h';
		}
	}

	ctx.AddSlice(cur - buffer, buffer, true);
	cur = buffer;
	size_t numLen = uitoa(value, cur, base);

	int precision = placeholder.dot.GetValue(0);
	if(precision < 0)
		throw syntax_exception("precision must be bigger than 0.", ctx.argId);
	if(numLen < (size_t)precision)
		PutZeros(ctx, precision - numLen);

	ctx.AddSlice(numLen, buffer, true);
}

template <typename T>
void conv_data_integer(Context& ctx, bool sign, T value, Placeholder& placeholder)
{
	if(placeholder.type == 'a' || placeholder.type == 'd') {
		if(placeholder.at.IsDefault())
			throw syntax_exception("Missing value for @ option.", ctx.fstrLastArgPos);

		const int base = placeholder.at.GetValue(10);

		if(base < 2 || base > 36)
			throw invalid_placeholder_value("Base for integer must be between 2 and 36.", ctx.fstrLastArgPos, base);
		write_integer(ctx, base, sign, value, placeholder);
	} else if(placeholder.type == 'h') {
		write_integer(ctx, 16, sign, value, placeholder);
	} else if(placeholder.type == 'c') {
		if(sign)
			throw value_exception("C placeholder can only print positive codepoints.", ctx.fstrLastArgPos);
		if(placeholder.hash.IsEnabled()) {
			AddCharLong(ctx, (uint32_t)value);
		} else {
			if(value != 0) {
				uint8_t utf8[6];
				int count = CodePointToUtf8((uint32_t)value, utf8);
				ctx.AddSlice(count, (const char*)utf8, true);
			}
		}
	} else {
		throw invalid_placeholder_type("Invalid placeholder for integer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void fmtPrint(Context& ctx, char data, Placeholder& placeholder)
{
	conv_data_integer<int>(ctx, data < 0, abs(data), placeholder);
}

void fmtPrint(Context& ctx, signed char data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, data < 0, abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed short data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, data < 0, abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed int data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, data < 0, abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed long data, Placeholder& placeholder)
{
	conv_data_integer<unsigned long>(ctx, data < 0, abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed long long data, Placeholder& placeholder)
{
	conv_data_integer<unsigned long long>(ctx, data < 0, abs(data), placeholder);
}

void fmtPrint(Context& ctx, unsigned char data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned short data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned int data, Placeholder& placeholder)
{
	conv_data_integer<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned long data, Placeholder& placeholder)
{
	conv_data_integer<unsigned long>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned long long data, Placeholder& placeholder)
{
	conv_data_integer<unsigned long long>(ctx, false, data, placeholder);
}

void fmtPrint(Context& ctx, const void* data, Placeholder& placeholder)
{
	if(placeholder.type == 'a' || placeholder.type == 'p')
		write_integer(ctx, 16, false, (uintptr_t)data, placeholder);
	else
		throw invalid_placeholder_type("Invalid placeholder for pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
}

void fmtPrint(Context& ctx, double data, Placeholder& placeholder)
{
	if(placeholder.dot.IsDefault())
		throw syntax_exception("Dot option is missing value.");

	auto& facet = ctx.GetLocale()->GetNumericalFacet();

	int digits = placeholder.dot.GetValue(facet.DefaultDigitCount);

	if(digits <= 0 || digits > 10)
		throw syntax_exception();

	bool forcePrecision = placeholder.star.IsEnabled();

	if(placeholder.type == 'a' || placeholder.type == 'f') {
		bool sign = (data < 0);
		if(sign || placeholder.plus)
			ctx.AddSlice(sign ? facet.Minus : facet.Plus);

		ftoa(ctx, sign ? -data : data, digits, forcePrecision, facet);
	} else if(placeholder.type == 'x') {
		if(placeholder.hash)
			ctx.AddSlice("0x");

		bool sign = (data < 0);
		if(sign || placeholder.plus)
			ctx.AddSlice(sign ? facet.Minus : facet.Plus);
		hftoa(ctx, sign ? -data : data, facet);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for floating-point type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void fmtPrint(Context& ctx, float data, Placeholder& placeholder)
{
	fmtPrint(ctx, (double)data, placeholder);
}
void fmtPrint(Context& ctx, long double data, Placeholder& placeholder)
{
	fmtPrint(ctx, (double)data, placeholder);
}

void fmtPrint(Context& ctx, bool data, Placeholder& placeholder)
{
	if(placeholder.type == 'a') {
		auto& facet = ctx.GetLocale()->GetBooleanFacet();
		ctx.AddSlice(data ? facet.True : facet.False);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for bool type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void fmtPrint(Context& ctx, Cursor* ptr, Placeholder& placeholder)
{
	if(!ptr)
		throw value_exception("Cursor pointer must be non-null.", ctx.fstrLastArgPos, ctx.argId);

	if(placeholder.type == 'n') {
		ptr->line = ctx.GetLine();
		ptr->collumn = ctx.GetCollumn() + 1;
	} else {
		throw invalid_placeholder_type("Invalid placeholder for cursor pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

size_t IntToString(intmax_t data, char* str, int base)
{
	int sign = (data < 0);
	if(sign) {
		*str++ = '-';
		data = -data;
	}
	size_t len = uitoa<uintmax_t>(data, str, base) + sign;
	str[len] = 0;
	return len;
}

size_t UIntToString(uintmax_t data, char* str, int base)
{
	size_t len = uitoa(data, str, base);
	str[len] = 0;
	return len;
}

size_t FloatToString(double data, char* str, int precision)
{
	if(precision < 1)
		precision = 3;
	if(precision > 10)
		precision = 10;

	size_t len = ftoaSimple(data, precision, str);
	str[len] = 0;
	return len;
}

}
