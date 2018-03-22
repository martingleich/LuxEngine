#include "format/ConvInternal.h"
#include "format/UnicodeConversion.h"
#include <cmath>

namespace format
{
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

size_t ftoaSimple(double n, int digits, char* str)
{
	if(digits < 0 || digits >= 10)
		return 0;

	char* old = str;

	if(std::isnan(n)) {
		*str++ = 'n';
		*str++ = 'a';
		*str++ = 'n';
	} else if(std::isinf(n)) {
		if(n < 0)
			*str++ = '-';
		*str++ = 'i';
		*str++ = 'n';
		*str++ = 'f';
	} else if(n == 0) {
		*str++ = '0';
	} else {
		int sign = (n < 0);
		if(sign)
			n = -n;

		if(sign)
			*str++ = '-';

		// 64-Bit since we need enough room for at least 13 digits.
		// otherwise we use exponential notation.
		uint64_t pre = (uint64_t)n;
		uint64_t post = (uint64_t)((n - std::floor(n)) * getPow10(digits + 1));

		size_t len = 0;
		do {
			*str++ = '0' + pre % 10;
			++len;
		} while((pre /= 10) > 0);

		reverse(str - len, len);

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
			*str++ = '.';

			while(post > 0) {
				*str++ = '0' + post % 10;
				post /= 10;
				++len;
			}

			while(len < post_digits) {
				*str++ = '0';
				++len;
			}

			reverse(str - len, len);
		}
	}

	return str - old;
}

void ftoa(Context& ctx, double n, int digits, bool forcePrecision, const Facet_NumericalFormat& locale)
{
	if(digits < 0 || digits >= 10)
		return;

	char BUFFER[32];
	char* s = BUFFER;

	if(std::isnan(n)) {
		ctx.AddSlice(locale.NaN);
	} else if(std::isinf(n)) {
		if(n < 0)
			ctx.AddSlice(locale.Minus);

		ctx.AddSlice(locale.Inf);
	} else if(n == 0) {
		ctx.AddSlice(1, "0");
		if(forcePrecision && digits > 0) {
			ctx.AddSlice(locale.Comma);
			PutZeros(ctx, digits);
		}
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
			ctx.AddSlice(locale.Minus);

		// 64-Bit since we need enough room for at least 13 digits.
		// otherwise we use exponential notation.
		uint64_t pre = (uint64_t)n;
		uint64_t post = (uint64_t)((n - std::floor(n)) * getPow10(digits + 1));

		// Round the number
		size_t post_digits = digits;
		if(post > 0) {
			if(post % 10 >= 5)
				post += 5;
			post /= 10;
			while(post % 10 == 0 && post) {
				post /= 10;
				--post_digits;
			}
			if(post_digits == 0) {
				pre++;
				post = 0;
			}
		}

		len = 0;
		do {
			*s++ = '0' + pre % 10;
			++len;
		} while((pre /= 10) > 0);

		reverse(s - len, len);
		ctx.AddSlice(s - BUFFER, BUFFER, true);
		s = BUFFER;

		bool hasPost = (post > 0);
		len = 0;
		if(hasPost) {
			ctx.AddSlice(locale.Comma);

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

		ctx.AddSlice(s - BUFFER, BUFFER, true);
		s = BUFFER;

		size_t rem = digits - len;
		if(forcePrecision && rem) {
			if(!hasPost)
				ctx.AddSlice(locale.Comma);
			while(rem--)
				*s++ = '0';
		}

		ctx.AddSlice(s - BUFFER, BUFFER, true);
		s = BUFFER;

		len = 0;
		if(useExp) {
			ctx.AddSlice(1, "e");
			if(m >= 0) {
				ctx.AddSlice(locale.Plus);
			} else {
				ctx.AddSlice(locale.Minus);
				m = -m;
			}

			while(m > 0) {
				*s++ = '0' + m % 10;
				m /= 10;
				++len;
			}

			reverse(s - len, len);
		}

		ctx.AddSlice(s - BUFFER, BUFFER, true);
	}
}

void hftoa(Context& ctx, double n, const Facet_NumericalFormat& locale)
{
	if(std::isnan(n)) {
		ctx.AddSlice(locale.NaN);
	} else if(std::isinf(n)) {
		if(n < 0)
			ctx.AddSlice(locale.Minus);

		ctx.AddSlice(locale.Inf);
	} else if(n == 0) {
		ctx.AddSlice(1, "0");
	} else {
		char BUFFER[64];
		uint64_t hex;
		memcpy(&hex, &n, sizeof(double));
		int32_t e = ((hex & 0x7FF0000000000000) >> 52) - ((1 << 10) - 1);
		uint64_t m = (hex & 0x000FFFFFFFFFFFFF);

		char* c = BUFFER;
		if(e != -1022)
			ctx.AddSlice(1, "1");
		else
			ctx.AddSlice(1, "0");

		if(m != 0) {
			ctx.AddSlice(locale.Comma);

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
		ctx.AddSlice(c - BUFFER, BUFFER, true);
		c = BUFFER;

		if(e >= 0) {
			ctx.AddSlice(locale.Plus);
		} else {
			ctx.AddSlice(locale.Minus);
			e = -e;
		}
		c += uitoa(e, c, 10);
		ctx.AddSlice(c - BUFFER, BUFFER);
	}
}

void AddCharLong(Context& ctx, uint32_t c)
{
	if(c == 127) {
		ctx.AddTerminatedSlice("<del");
	} else if(c <= 32) {
		static const char* TABLE[] = {
			"<null>", "<soh>", "<stx>", "<etx>", "<eot>", "<enq>", "<ack>",
			"<bell>", "<backspace>", "<tab>", "<linefeed>", "<vt>",
			"<new page>", "<carrige return>", "<so>", "<si>",
			"<dle>", "<dc1>", "<dc2>", "<dc3>", "<dc4>", "<nak>",
			"<syn>", "<etb>", "<cancel>", "<em>", "<sub>", "<escape>",
			"<fs>", "<gs>", "<rs>", "<us>", "<space>"};

		ctx.AddTerminatedSlice(TABLE[c]);
	} else {
		uint8_t utf8[6];
		int count = CodePointToUtf8(c, utf8);
		ctx.AddSlice(count, (const char*)utf8, true);
	}
}

void PutCount(Context& ctx, size_t count, const char* buffer, size_t bufferSize)
{
	const char* ptr = buffer;
	AdvanceCursor(ptr);
	size_t charSize = ptr - buffer;
	auto bufferCount = bufferSize / charSize;

	while(count > 0) {
		if(count >= bufferCount) {
			ctx.AddSlice(bufferCount*charSize, buffer);
			count -= bufferCount;
		} else {
			ctx.AddSlice(count*charSize, buffer);
			break;
		}
	}
}

void PutSpaces(Context& ctx, size_t count)
{
	static const char* SPACES = "                                "; // 32 Spaces
	PutCount(ctx, count, SPACES, 32);
}

void PutZeros(Context& ctx, size_t count)
{
	static const char* ZEROS = "00000000000000000000000000000000"; // 32 Zeros
	PutCount(ctx, count, ZEROS, 32);
}
}
