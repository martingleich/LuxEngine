#include "Converters.h"
#include "FormatConfig.h"
#include "StringBasics.h"

namespace format
{
void conv_data(Context& ctx, const char* data, Placeholder& placeholder)
{
	if(placeholder.type == 'a' || placeholder.type == 's') {
		if(data) {
			size_t len = StringLengthFixed<uint8_t>(data);
			ConvertAddString(ctx, StringType::FORMAT_STRING_TYPE, data, len);
		} else {
			throw value_exception("Can only print non-null string pointers.", ctx.fstrLastArgPos, ctx.argId);
		}
	} else {
		throw invalid_placeholder_type("Invalid placeholder for char string pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void conv_data(Context& ctx, const std::string& data, Placeholder& placeholder)
{
	(void)ctx;

	if(placeholder.type == 'a' || placeholder.type == 's') {
		ConvertAddString(ctx, StringType::FORMAT_STRING_TYPE, data.c_str(), data.length());
	} else {
		throw invalid_placeholder_type("Invalid placeholder for std::string type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void write_integer(Context& ctx, int base, bool sign, uintmax_t value, Placeholder& placeholder)
{
	auto& facet = ctx.GetLocale()->GetNumericalFacet();

	if(sign || placeholder.plus) {
		const char* str = sign ? facet.Minus : facet.Plus;
		ConvertAddString(ctx, StringType::Unicode, str, strlen(str));
	}

	char buffer[64];
	size_t len = 0;
	char* cur = buffer;

	if(placeholder.hash) {
		if(base == 2) {
			*cur++ = 'b';
			++len;
		} else if(base == 8) {
			*cur++ = 'o';
			++len;
		} else if(base == 16) {
			*cur++ = 'h';
			++len;
		}
	}

	len += internal::uitoa(value, cur, base);

	CopyConvertAddString(ctx, StringType::Ascii, buffer, len);
}

void conv_data_integer(Context& ctx, bool sign, uintmax_t value, Placeholder& placeholder)
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
			internal::AddCharLong(ctx, (uint32_t)value);
		} else {
			if(value != 0) {
				uint8_t utf8[6];
				int count = CodePointToUtf8((uint32_t)value, utf8);
				CopyConvertAddString(ctx, StringType::Unicode, (const char*)utf8, count);
			}
		}
	} else {
		throw invalid_placeholder_type("Invalid placeholder for integer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void conv_data(Context& ctx, intmax_t data, Placeholder& placeholder)
{
	conv_data_integer(ctx, data < 0, abs(data), placeholder);
}

void conv_data(Context& ctx, uintmax_t data, Placeholder& placeholder)
{
	conv_data_integer(ctx, false, data, placeholder);
}

void conv_data(Context& ctx, const void* data, Placeholder& placeholder)
{
	if(placeholder.type == 'a' || placeholder.type == 'p')
		write_integer(ctx, 16, false, (uintptr_t)data, placeholder);
	else
		throw invalid_placeholder_type("Invalid placeholder for pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
}

void conv_data(Context& ctx, double data, Placeholder& placeholder)
{
	if(placeholder.dot.IsDefault())
		throw syntax_exception("Dot option is missing value.");

	int digits = placeholder.dot.GetValue(4);

	if(digits <= 0 || digits > 10)
		throw syntax_exception();

	bool forcePrecision = placeholder.star.IsEnabled();

	auto& facet = ctx.GetLocale()->GetNumericalFacet();

	if(placeholder.type == 'a' || placeholder.type == 'f') {
		bool sign = (data < 0);
		if(sign || placeholder.plus) {
			const char* str = sign ? facet.Minus : facet.Plus;
			ConvertAddString(ctx, StringType::Unicode, str, strlen(str));
		}

		internal::ftoa(ctx, sign ? -data : data, digits, forcePrecision, facet);
	} else if(placeholder.type == 'x') {
		if(placeholder.hash)
			ConvertAddString(ctx, StringType::Unicode, "0x", 2);

		bool sign = (data < 0);
		if(sign || placeholder.plus) {
			const char* str = sign ? facet.Minus : facet.Plus;
			ConvertAddString(ctx, StringType::Unicode, str, strlen(str));
		}
		internal::hftoa(ctx, sign ? -data : data, facet);
	} else{
		throw invalid_placeholder_type("Invalid placeholder for floating-point type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

void conv_data(Context& ctx, bool data, Placeholder& placeholder)
{
	const char* buffer;
	size_t len;
	if(placeholder.type == 'a') {
		buffer = ctx.GetLocale()->GetBooleanFacet().GetString(data);
		len = strlen(buffer);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for bool type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}

	ctx.AddSlice(ConvertString(ctx, StringType::Unicode, buffer, len));
}

void conv_data(Context& ctx, Cursor* ptr, Placeholder& placeholder)
{
	if(!ptr)
		throw value_exception("Cursor pointer must be non-null.", ctx.fstrLastArgPos, ctx.argId);

	if(placeholder.type == 'n') {
		ptr->line = ctx.GetLine();
		ptr->count = ctx.GetCharacterCount();
		ptr->collumn = ctx.GetCollumn() + 1;
	} else {
		throw invalid_placeholder_type("Invalid placeholder for crusor pointer type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

}
