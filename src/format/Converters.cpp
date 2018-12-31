#include "format/Converters.h"
#include "format/ConvertersHelper.h"
#include "format/UnicodeConversion.h"

namespace format
{

static int CodePointToUtf8(uint32_t c, uint8_t* out)
{
	uint8_t* orig = out;
	if(c <= 0x7F) {
		*out++ = ((uint8_t)c);
	} else if(c <= 0x7FF) {
		*out++ = ((uint8_t)(0xC0 | ((c&(0x1F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0xFFFF) {
		*out++ = ((uint8_t)(0xE0 | ((c & (0xF << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x1FFFFF) {
		*out++ = ((uint8_t)(0xF0 | ((c&(0x7 << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3f))));
	} else if(c <= 0x3FFFFFF) {
		*out++ = ((uint8_t)(0xF8 | ((c&(0x3 << 24)) >> 24)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x7FFFFFFF) {
		*out++ = ((uint8_t)(0xFC | ((c&(0x1 << 30)) >> 30)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 24)) >> 24)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else {
		*out++ = 0;
	}

	return (int)(out - orig);
}

static void PutCharacter(Context& ctx, bool sign, uint32_t value, bool longForm)
{
	if(sign)
		throw invalid_argument("C placeholder can only print positive codepoints.", ctx.GetCurArgId());
	if(longForm) {
		if(value == 127) {
			ctx.AddTerminatedSlice("<del");
		} else if((uint32_t)value <= 32) {
			static const char* TABLE[] = {
				"<null>", "<soh>", "<stx>", "<etx>", "<eot>", "<enq>", "<ack>",
				"<bell>", "<backspace>", "<tab>", "<linefeed>", "<vt>",
				"<new page>", "<carrige return>", "<so>", "<si>",
				"<dle>", "<dc1>", "<dc2>", "<dc3>", "<dc4>", "<nak>",
				"<syn>", "<etb>", "<cancel>", "<em>", "<sub>", "<escape>",
				"<fs>", "<gs>", "<rs>", "<us>", "<space>"};

			ctx.AddTerminatedSlice(TABLE[(uint32_t)value]);
		} else {
			uint8_t utf8[6];
			int count = CodePointToUtf8((uint32_t)value, utf8);
			ctx.AddSlice(count, (const char*)utf8, true);
		}
	} else {
		if(value != 0) {
			uint8_t utf8[6];
			int count = CodePointToUtf8((uint32_t)value, utf8);
			ctx.AddSlice(count, (const char*)utf8, true);
		}
	}
}

template <typename T>
static void PutIntTempl(Context& ctx, bool sign, T value, const Placeholder& placeholder)
{
	auto pl = parser::BasicPlaceholder::Parse(placeholder.format, ctx, ctx.GetCurArgId());
	if(placeholder.type == 'h' || placeholder.type == 'd' || placeholder.type.size == 0) {
		int base = pl.at.GetValue(placeholder.type == 'h' ? 16 : 10);
		int precision = pl.dot.GetValue(0);
		bool forceSign = pl.plus.IsEnabled();
		bool prefix = pl.hash.IsEnabled();
		auto& facet = ctx.GetLocale()->GetNumericalFacet();
		ifconst(sizeof(T) <= sizeof(unsigned int))
			PutInt(ctx, unsigned int(value), sign, forceSign, precision, base, prefix, facet);
		else ifconst(sizeof(T) <= sizeof(unsigned long))
			PutLong(ctx, unsigned long(value), sign, forceSign, precision, base, prefix, facet);
		else
			PutLongLong(ctx, unsigned long long(value), sign, forceSign, precision, base, prefix, facet);
	} else if(placeholder.type == 'c') {
		PutCharacter(ctx, sign, uint32_t(value), pl.hash.IsEnabled());
	} else {
		throw invalid_placeholder_type("Invalid placeholder for integer type.", ctx.GetCurPlaceholderOffset());
	}
}

void fmtPrint(Context& ctx, const char* data, const Placeholder& placeholder)
{
	if(placeholder.type.size == 0) {
		if(data)
			ctx.AddTerminatedSlice(data, true);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for char string pointer type.", ctx.GetCurPlaceholderOffset());
	}
}

void fmtPrint(Context& ctx, char data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, data < 0, std::abs(data), placeholder);
}

void fmtPrint(Context& ctx, signed char data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, data < 0, std::abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed short data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, data < 0, std::abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed int data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, data < 0, std::abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed long data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned long>(ctx, data < 0, std::abs(data), placeholder);
}
void fmtPrint(Context& ctx, signed long long data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned long long>(ctx, data < 0, abs(data), placeholder);
}

void fmtPrint(Context& ctx, unsigned char data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned short data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned int data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned int>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned long data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned long>(ctx, false, data, placeholder);
}
void fmtPrint(Context& ctx, unsigned long long data, const Placeholder& placeholder)
{
	PutIntTempl<unsigned long long>(ctx, false, data, placeholder);
}

void fmtPrint(Context& ctx, const void* data, const Placeholder& placeholder)
{
	if(placeholder.type.size == 0) {
		auto pl = parser::BasicPlaceholder::Parse(placeholder.format, ctx, ctx.GetCurArgId());
		PutLongLong(ctx, (unsigned long long)data, false, false, pl.dot.GetValue(sizeof(void*) * 2), 16, pl.hash.IsEnabled(), ctx.GetLocale()->GetNumericalFacet());
	} else {
		throw invalid_placeholder_type("Invalid placeholder for pointer type.", ctx.GetCurPlaceholderOffset());
	}
}

void fmtPrint(Context& ctx, double data, const Placeholder& placeholder)
{
	auto pl = parser::BasicPlaceholder::Parse(placeholder.format, ctx, ctx.GetCurArgId());
	if(pl.dot.IsEnabled() && !pl.dot.HasValue())
		throw syntax_exception("Dot option is missing value.", ctx.GetCurPlaceholderOffset());

	auto& facet = ctx.GetLocale()->GetNumericalFacet();

	int digits = pl.dot.GetValue(facet.DefaultDigitCount);

	if(digits <= 0 || digits > 10)
		throw invalid_placeholder_value("Invalid digit count", ctx.GetCurPlaceholderOffset(), digits);

	bool forcePrecision = pl.star.IsEnabled();

	if(placeholder.type == 'f' || placeholder.type.size == 0) {
		bool sign = (data < 0);
		if(sign || pl.plus.IsEnabled())
			ctx.AddSlice(sign ? facet.Minus : facet.Plus);

		PutFloat(ctx, sign ? -data : data, digits, forcePrecision, facet);
	} else if(placeholder.type == 'h') {
		if(pl.hash.IsEnabled())
			ctx.AddSlice(1, "h");

		bool sign = (data < 0);
		if(sign || pl.plus.IsEnabled())
			ctx.AddSlice(sign ? facet.Minus : facet.Plus);
		PutHexFloat(ctx, sign ? -data : data, facet);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for floating-point type.", ctx.GetCurPlaceholderOffset());
	}
}

void fmtPrint(Context& ctx, float data, const Placeholder& placeholder)
{
	fmtPrint(ctx, (double)data, placeholder);
}

void fmtPrint(Context& ctx, long double data, const Placeholder& placeholder)
{
	fmtPrint(ctx, (double)data, placeholder);
}

void fmtPrint(Context& ctx, bool data, const Placeholder& placeholder)
{
	if(placeholder.type == 'b' || placeholder.type.size == 0) {
		auto& facet = ctx.GetLocale()->GetBooleanFacet();
		ctx.AddSlice(data ? facet.True : facet.False);
	} else {
		throw invalid_placeholder_type("Invalid placeholder for bool type.", ctx.GetCurPlaceholderOffset());
	}
}

void fmtPrint(Context& ctx, Cursor* ptr, const Placeholder& placeholder)
{
	if(!ptr)
		throw invalid_argument("Cursor pointer must be non-null.", ctx.GetCurArgId());

	if(placeholder.type == 'n' || placeholder.type.size == 0)
		ptr->pos = ctx.GetSize();
	else
		throw invalid_placeholder_type("Invalid placeholder type for cursor pointer type.", ctx.GetCurPlaceholderOffset());
}

}
