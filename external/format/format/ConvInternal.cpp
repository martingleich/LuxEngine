#include "ConvInternal.h"
#include "StringBasics.h"
#include <cmath>

namespace format
{
namespace internal
{
	static slice ConvertFixed(size_t dstByte, char* dst, size_t srcByte, const char* src, size_t srcSize)
	{
		assert(srcSize%srcByte == 0);

		char* base = dst;
		for(size_t i = 0; i < srcSize; i += srcByte) {
			uint32_t x;
			memcpy(&x, src, srcByte);
			memcpy(dst, &x, dstByte);
			src += srcByte;
			dst += dstByte;
		}

		return slice((srcSize / srcByte) * dstByte, base);
	}

	// Reverse a string.
	void reverse(char* s, size_t len)
	{
		char* b = s + len - 1;
		while(s < b) {
			char c = *s;
			*s++ = *b;
			*b-- = c;
		}
	}

	static double getPow10(int i)
	{
		static const double table[23] = {
			0.00000000001,
			0.0000000001,
			0.000000001,
			0.00000001,
			0.0000001,
			0.000001,
			0.00001,
			0.0001,
			0.001,
			0.01,
			0.1,
			1.0,
			10.0,
			100.0,
			1000.0,
			10000.0,
			100000.0,
			1000000.0,
			10000000.0,
			100000000.0,
			1000000000.0,
			10000000000.0,
			100000000000.0,
		};

		if(i >= -11 && i <= 11)
			return table[i + 11];
		else
			return pow(10, i);
	}

	//! Convert a floating point number to a string.
	/**
	for Not a number nan is written
	for infinite +inf or -inf is written.
	For number bigger than 10^13 or smaller than 10^-9 expontial notation is used in format 12.34e+5
	\param n The number to convert
	\param [out] s Here the resulting string is written. Must be at least MAX_DOUBLE_LEN+digits character long, String is not NULL-terminated.
	\param digiti The maximal number of digits after the decimal-point.
	\return The length of the resulting string, 0 if an error occured.
	*/
	void ftoa(Context& ctx, double n, int digits, bool forcePrecision, const locale::Facet_NumericalFormat& locale)
	{
		if(digits < 0 || digits >= 10)
			return;

		char BUFFER[32];
		char* s = BUFFER;

		if(std::isnan(n)) {
			ConvertAddString(ctx, StringType::Ascii, locale.NaN, strlen(locale.NaN));
		} else if(std::isinf(n)) {
			if(n < 0)
				ConvertAddString(ctx, StringType::Unicode, locale.Minus, strlen(locale.Minus));

			ConvertAddString(ctx, StringType::Ascii, locale.Inf, strlen(locale.Inf));
		} else if(n == 0) {
			ConvertAddString(ctx, StringType::Ascii, "0", 1);
		} else {
			int m;
			size_t len;
			int sign = (n < 0);
			if(sign)
				n = -n;
			m = (int)std::log10(n);
			int useExp = (m >= 14 || (sign && m >= 9) || m <= -9);
			if(useExp) {
				if(m < 0)
					m -= 1;
				n /= getPow10(m);
			}

			if(sign)
				ConvertAddString(ctx, StringType::Unicode, locale.Minus, strlen(locale.Minus));

			// 64-Bit since we need enough room for at least 13 digits.
			// otherwise we use exponential notation.
			uint64_t pre = (uint64_t)n;
			uint64_t post = (uint64_t)((n - floor(n)) * getPow10(digits + 1));


			len = 0;
			do {
				*s++ = '0' + pre % 10;
				++len;
			} while((pre /= 10) > 0);

			reverse(s - len, len);
			CopyConvertAddString(ctx, StringType::Unicode, BUFFER, s - BUFFER);
			s = BUFFER;

			bool hasPost = (post > 0);
			len = 0;
			if(hasPost) {
				size_t post_digits = digits;
				if(post % 10 >= 5)
					post += 5;
				post /= 10;
				while(post % 10 == 0 && post) {
					post /= 10;
					--post_digits;
				}
				ConvertAddString(ctx, StringType::Unicode, locale.Comma, strlen(locale.Comma));

				while(post > 0) {
					*s++ = '0' + post % 10;
					post /= 10;
					++len;
				}

				while(len < post_digits) {
					*s++ = '0';
					++len;
				}

				reverse(s - len, len);
			}

			CopyConvertAddString(ctx, StringType::Unicode, BUFFER, s - BUFFER);
			s = BUFFER;

			size_t rem = digits - len;
			if(forcePrecision && rem) {
				if(!hasPost)
					ConvertAddString(ctx, StringType::Unicode, locale.Comma, strlen(locale.Comma));
				while(rem--)
					*s++ = '0';
			}

			CopyConvertAddString(ctx, StringType::Unicode, BUFFER, s - BUFFER);
			s = BUFFER;

			len = 0;
			if(useExp) {
				CopyConvertAddString(ctx, StringType::Unicode, "e", 1);
				if(m >= 0) {
					ConvertAddString(ctx, StringType::Unicode, locale.Plus, strlen(locale.Plus));
				} else {
					ConvertAddString(ctx, StringType::Unicode, locale.Minus, strlen(locale.Minus));
					m = -m;
				}

				while(m > 0) {
					*s++ = '0' + m % 10;
					m /= 10;
					++len;
				}

				reverse(s - len, len);
			}

			CopyConvertAddString(ctx, StringType::Unicode, BUFFER, s - BUFFER);
		}
	}

	void hftoa(Context& ctx, double n, const locale::Facet_NumericalFormat& locale)
	{
		if(std::isnan(n)) {
			ConvertAddString(ctx, StringType::Ascii, locale.NaN, strlen(locale.NaN));
		} else if(std::isinf(n)) {
			if(n < 0)
				ConvertAddString(ctx, StringType::Unicode, locale.Minus, strlen(locale.Minus));

			ConvertAddString(ctx, StringType::Ascii, locale.Inf, strlen(locale.Inf));
		} else if(n == 0) {
			ConvertAddString(ctx, StringType::Ascii, "0", 1);
		} else {
			char BUFFER[64];
			uint64_t hex;
			memcpy(&hex, &n, sizeof(double));
			int32_t e = ((hex & 0x7FF0000000000000) >> 52) - ((1 << 10) - 1);
			uint64_t m = (hex & 0x000FFFFFFFFFFFFF);

			char* c = BUFFER;
			if(e != -1022)
				CopyConvertAddString(ctx, StringType::Ascii, "1", 1);
			else
				CopyConvertAddString(ctx, StringType::Ascii, "0", 1);

			if(m != 0) {
				ConvertAddString(ctx, StringType::Unicode, locale.Comma, strlen(locale.Comma));

				int i = 12;
				uint64_t d = 0xF000000000000;
				while(m) {
					char r = (char)((m & d) >> (4 * i));
					m &= ~d;
					*c++ = r < 10 ? '0' + r : 'A' + r - 10;
					d >>= 4;
					i--;
				}
			}
			*c++ = 'p';
			CopyConvertAddString(ctx, StringType::Ascii, BUFFER, c - BUFFER);
			c = BUFFER;

			if(e >= 0) {
				ConvertAddString(ctx, StringType::Unicode, locale.Plus, strlen(locale.Plus));
			} else {
				ConvertAddString(ctx, StringType::Unicode, locale.Minus, strlen(locale.Minus));
				e = -e;
			}
			c += uitoa(e, c, 10);
			CopyConvertAddString(ctx, StringType::Ascii, BUFFER, c - BUFFER);
		}
	}

	//! Convert a character to it's long representation
	/**
	A characters long represenation is the normal character for simple letters digits or signs.
	For special or invisible character like space, backspace, null or linefeed it's a describing string
	(i.e. <space>, <backspace>, <null>, <linefeed>). <br>
	Allocates memory from the Context.
	\param ctx The format context, no visible changes are made to it.
	\param c The character to convert
	\return A slice for the long form.
	*/
	void AddCharLong(Context& ctx, uint32_t c)
	{
		if(c == 127) {
			ConvertAddString(ctx, StringType::Ascii, "<del>", 5);
		} else if(c <= 32) {
			static const char* TABLE[] = {
				"<null>", "<soh>", "<stx>", "<etx>", "<eot>", "<enq>", "<ack>",
				"<bell>", "<backspace>", "<tab>", "<linefeed>", "<vt>",
				"<new page>", "<carrige return>", "<so>", "<si>",
				"<dle>", "<dc1>", "<dc2>", "<dc3>", "<dc4>", "<nak>",
				"<syn>", "<etb>", "<cancel>", "<em>", "<sub>", "<escape>",
				"<fs>", "<gs>", "<rs>", "<us>", "<space>"};

			ConvertAddString(ctx, StringType::Ascii, TABLE[c], strlen(TABLE[c]));
		} else {
			uint8_t utf8[6];
			int count = CodePointToUtf8(c, utf8);
			CopyConvertAddString(ctx, StringType::Unicode, (const char*)utf8, count);
		}
	}

	void PutCount(Context& ctx, size_t count, StringType type, const char* buffer, size_t maxCount)
	{
		if(ctx.stringType != type) {
			slice x = ConvertString(ctx, type, buffer, maxCount);
			buffer = x.data;
		}

		while(count > 0) {
			if(count >= maxCount) {
				ctx.AddSlice(maxCount, buffer);
				count -= maxCount;
			} else {
				ctx.AddSlice(count, buffer);
				return;
			}
		}
	}

	void PutSpaces(Context& ctx, size_t count)
	{
		static const char* SPACES = "                                "; // 32 Spaces
		PutCount(ctx, count, StringType::Ascii, SPACES, sizeof(SPACES));
	}

	void FormatTilde(Context& ctx, const Placeholder& placeholder)
	{
		static const char* TILDES = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"; // 32 Tildes

		const int count = placeholder.master.GetValue(0);

		if(count == 0)
			return;

		if(count < 0)
			throw invalid_placeholder_value("Number of tildes must be bigger than zero.", ctx.fstrLastArgPos, count);

		PutCount(ctx, count, StringType::Ascii, TILDES, sizeof(TILDES));
	}

	void FormatTab(Context& ctx, const Placeholder& placeholder)
	{
		if(!placeholder.master.HasValue())
			throw value_exception("Tab placeholder requires argument.");
		const int tab_stop = placeholder.master.GetValue();

		if(placeholder.hash) {
			if(tab_stop < 0)
				throw value_exception("Tab placeholder value must be bigger than 0.");

			PutSpaces(ctx, tab_stop);
		} else {
			if(tab_stop < 1)
				throw value_exception("Tab placeholder value must be bigger than 0.");
			if(tab_stop == 1)
				return;

			size_t p = ctx.GetCollumn() + 1;
			if(!placeholder.plus && p == 1)
				return;
			if(p%tab_stop == 0)
				return;

			size_t count = ((p / tab_stop + 1)*tab_stop - p);

			PutSpaces(ctx, count);
		}
	}

	bool TryFormatArgFree(Context& ctx, Placeholder& placeholder)
	{
		switch(placeholder.type) {
		case '~':
			FormatTilde(ctx, placeholder);
			break;
		case 't':
			FormatTab(ctx, placeholder);
			break;
		default:
			return false;
		}

		return true;
	}
}

slice ConvertString(Context& dst, StringType srcType, const char* srcData, size_t srcSize)
{
	if(srcType == dst.stringType)
		return slice(srcSize, srcData);

	if(srcType == StringType::Ascii) {
		if(dst.stringType == StringType::Unicode)
			return slice(srcSize, srcData);

		size_t dstByte = GetBytePerChar(dst.stringType);
		size_t srcByte = GetBytePerChar(srcType);
		char* data = dst.AllocByte(srcSize*dstByte);
		return internal::ConvertFixed(dstByte, data, srcByte, srcData, srcSize);
	}

	if(srcType == StringType::Unicode) {
		if(dst.stringType == StringType::Ascii) {
			const uint8_t* s = (const uint8_t*)srcData;
			const uint8_t* e = (const uint8_t*)srcData + srcSize;
			char* data = dst.AllocByte(srcSize);
			char* base = data;
			while(s < e) {
				if((*s & 0xC0) != 0x80)
					*data++ = ((*s&(uint8_t)0x80) == 0) ? (*s&(0x7F)) : '?';
				++s;
			}

			size_t size = data-base;
			if(size == srcSize) {
				dst.UnallocByte(base);
				return slice(size, srcData);
			}

			return slice(data-base, base);
		}
	}

	throw not_implemeted_exception("Can't cast string format");
}

slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize)
{
	slice x = ConvertString(dst, srcType, srcData, srcSize);
	if(x.size != 0)
		return dst.AddSlice(x);
	else
		return nullptr;
}

slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize, const Cursor& curDiff)
{
	slice x = ConvertString(dst, srcType, srcData, srcSize);
	if(x.size != 0)
		return dst.AddSlice(x.size, x.data, false, &curDiff);
	else
		return nullptr;
}

slice* CopyConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize)
{
	slice x = ConvertString(dst, srcType, srcData, srcSize);
	if(x.size != 0) {
		return dst.AddSlice(x.size, x.data, (x.data == srcData));
	} else
		return nullptr;
}

}
