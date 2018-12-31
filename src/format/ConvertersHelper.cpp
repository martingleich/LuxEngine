#include "format/ConvertersHelper.h"
#include "format/UnicodeConversion.h"
#include "format/GeneralParsing.h"
#include <cmath>

namespace format
{
static void ReverseString(char* s, int len)
{
	char* b = s + len - 1;
	while(s < b) {
		char c = *s;
		*s++ = *b;
		*b-- = c;
	}
}

static double GetPow10(int i)
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

// sign pre.postEexp
// MAX_INT_LEN + 1 + digits + 1 + MAX_INT_LEN
// 11 + 1 + 11 + 1= 24
//static size_t MAX_DOUBLE_LEN = 24;

//! Convert a unsigned interger to an ASCII string
template <typename T>
int IntToStringTempl(T n, char* s, int base)
{
	if(base > 10 + 26 || base < 2)
		return 0;

	const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char* c = s;
	// Record digits in inverse order
	do {
		auto digit = n % base;
		*c++ = digits[digit];
	} while((n /= base) > 0);

	// Reverse the rest.
	ReverseString(s, int(c - s));

	return int(c - s);
}

/*
for Not a number nan is written
for infinite +inf or -inf is written.
For number bigger than 10^13 or smaller than 10^-9 expontial notation is used in format 12.34e+5

The last rule implies that the precomma part has at most 13 digits.
*/
void PutFloat(Context& ctx, double n, int digits, bool forcePrecision, const Facet_NumericalFormat& locale)
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
		int len;
		int sign = (n < 0);
		if(sign)
			n = -n;

		// Check is exponential notation should be used.
		m = (int)std::log10(n);
		int useExp = (m >= 14 || (sign && m >= 9) || m <= -9);
		if(useExp) {
			if(m < 0)
				m -= 1;
			n /= GetPow10(m);
		}

		if(sign)
			ctx.AddSlice(locale.Minus);

		// Split into integer part before comma and after comma.
		// 64-Bit since we need enough room for at least 13 digits.
		// otherwise we use exponential notation.
		uint64_t pre = (uint64_t)n;
		uint64_t post = (uint64_t)((n - std::floor(n)) * GetPow10(digits + 1));

		// Round the number
		int post_digits = digits;
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

		// Write pre part
		len = 0;
		do {
			*s++ = '0' + pre % 10;
			++len;
		} while((pre /= 10) > 0);

		ReverseString(s - len, len);
		ctx.AddSlice(int(s - BUFFER), BUFFER, true);
		s = BUFFER;

		// Write post part
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

			ReverseString(s - len, len);
		}

		ctx.AddSlice(int(s - BUFFER), BUFFER, true);
		s = BUFFER;

		// Add zeros for precision.
		size_t rem = digits - len;
		if(forcePrecision && rem) {
			if(!hasPost)
				ctx.AddSlice(locale.Comma);
			while(rem--)
				*s++ = '0';
		}

		ctx.AddSlice(int(s - BUFFER), BUFFER, true);
		s = BUFFER;

		// Write exponent
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

			ReverseString(s - len, len);
		}

		ctx.AddSlice(int(s - BUFFER), BUFFER, true);
	}
}

static void GetExpFraction(double dvalue, int& exponent, uint64_t& fraction)
{
	static_assert(std::numeric_limits<double>::is_iec559, "Must be iec559");

	uint64_t value;
	std::memcpy(&value, &dvalue, 8);
	fraction = value & 0x0FFFFFFFFFFFFF;
	uint32_t expBase = (value >> 52) & 0x7FF;
	exponent = expBase - 1023;
}

void PutHexFloat(Context& ctx, double n, const Facet_NumericalFormat& locale)
{
	if(std::isnan(n)) {
		ctx.AddSlice(locale.NaN);
		return;
	}

	if(n < 0 || n == -0) {
		ctx.AddSlice(locale.Minus);
		n = -n;
	}

	if(std::isinf(n)) {
		ctx.AddSlice(locale.Inf);
	} else if(n == 0) {
		ctx.AddSlice(1, "0");
	} else {
		char BUFFER[64];
		int e;
		uint64_t m;
		GetExpFraction(n, e, m);

		char* c = BUFFER;
		if(e != -1023)
			ctx.AddSlice(1, "1"); // Normal
		else
			ctx.AddSlice(1, "0"); // Denormal

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
		ctx.AddSlice(int(c - BUFFER), BUFFER, true);
		c = BUFFER;

		if(e >= 0) {
			ctx.AddSlice(locale.Plus);
		} else {
			ctx.AddSlice(locale.Minus);
			e = -e;
		}

		c += IntToStringTempl(e, c, 10);
		ctx.AddSlice(int(c - BUFFER), BUFFER);
	}
}

static void PutIntPrefix(Context& ctx, int base)
{
	char pre = 0;
	if(base == 2)
		pre = 'b';
	else if(base == 8)
		pre = 'o';
	else if(base == 10)
		pre = 'd';
	else if(base == 16)
		pre = 'h';
	if(pre)
		ctx.AddSlice(1, &pre, true);
}

template <typename T>
void PutIntTempl(Context& ctx, T i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& facet)
{
	if(base < 2 || base > 36)
		throw invalid_placeholder_value("Base for integer must be between 2 and 36.", ctx.GetCurPlaceholderOffset(), base);
	if(precision < 0)
		throw syntax_exception("precision must be bigger or equal to 0.", ctx.GetCurArgId());

	if(sign || forceSign)
		ctx.AddSlice(sign ? facet.Minus : facet.Plus);

	if(prefix)
		PutIntPrefix(ctx, base);

	// Binary encoding is the longest so number of bits is the maximal length.
	char buffer[sizeof(T)*CHAR_BIT];
	int numLen = IntToStringTempl(i, buffer, base);

	if(numLen < precision)
		PutZeros(ctx, precision - numLen);

	ctx.AddSlice(numLen, buffer, true);
}

void PutInt(Context& ctx, unsigned int i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale)
{
	PutIntTempl<unsigned int>(ctx, i, sign, forceSign, precision, base, prefix, locale);
}

void PutLong(Context& ctx, unsigned long i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale)
{
	PutIntTempl<unsigned long>(ctx, i, sign, forceSign, precision, base, prefix, locale);
}

void PutLongLong(Context& ctx, unsigned long long i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale)
{
	PutIntTempl<unsigned long long>(ctx, i, sign, forceSign, precision, base, prefix, locale);
}

void PutCount(Context& ctx, int charSize, int count, const char* buffer, int bufferSize)
{
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

void PutSpaces(Context& ctx, int count)
{
	static const char* SPACES = "                                "; // 32 Spaces
	PutCount(ctx, 1, count, SPACES, 32);
}

void PutZeros(Context& ctx, int count)
{
	static const char* ZEROS = "00000000000000000000000000000000"; // 32 Zeros
	PutCount(ctx, 1, count, ZEROS, 32);
}

////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////

namespace parser
{
BasicPlaceholder BasicPlaceholder::Parse(Slice slice, Context& ctx, int baseArgId)
{
	int argid = baseArgId + 1;
	BasicPlaceholder out;
	SaveStringReader reader(slice);
	while(!reader.IsEnd()) {
		Option* ptr;
		switch(reader.Get()) {
		case '.': ptr = &out.dot; break;
		case '*': ptr = &out.star; break;
		case '#': ptr = &out.hash; break;
		case '+': ptr = &out.plus; break;
		case '@': ptr = &out.plus; break;
		default:
			throw syntax_exception("Unknown format element", (size_t)-1);
		}

		*ptr = Option(-2);
		if(!reader.IsEnd()) {
			if(reader.Peek() == '{') {
				// Flexible argument.
				reader.Get();
				int v;
				SkipSpace(reader);
				if(TryReadInteger(reader, v))
					argid = v;
				SkipSpace(reader);

				int newV = ctx.GetFormatEntry(argid)->AsInteger();
				if(newV <= 0)
					throw invalid_argument("Placeholder argument can't be negative", argid);
				*ptr = Option(newV);
				++argid;

				if(reader.IsEnd() || reader.Peek() != '}')
					throw syntax_exception("Unknown internal placeholder value", ctx.GetCurPlaceholderOffset());
				reader.Get();
			} else {
				// Constant argument
				int v;
				if(TryReadInteger(reader, v))
					*ptr = Option(v);
			}
		}
	}
	return out;
}
}
}
