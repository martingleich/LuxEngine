#include "core/lxUnicode.h"
#include "core/lxException.h"

extern "C"
{
#include <utf8proc/utf8proc.h>
}

namespace lux
{
namespace core
{

size_t StringLengthUTF8(const char* str, size_t* outBytes)
{
	size_t len = 0;
	const u8* s = (const u8*)str;
	while(*s) {
		if((*s & 0xC0) != 0x80)
			++len;
		++s;
	}
	if(outBytes)
		*outBytes = s - (const u8*)str;

	return len;
}

void RetractCursorUTF8(const char*& ptr)
{
	--ptr;
	while((*ptr & 0xC0) == 0x80)
		--ptr;
}

u32 AdvanceCursorUTF8(const char*& ptr)
{
	uint8_t u0 = *ptr++;
	if((u0 & 0x80) == 0) // 0xxxxxxx 
		return (uint32_t)u0;
	uint8_t u1 = *ptr++;
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	uint8_t u2 = *ptr++;
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	uint8_t u3 = *ptr++;
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC0) << 0;

	lxAssertNeverReach("Invalid utf8-codepoint");
}

u32 GetCharacterUTF8(const char* ptr)
{
	uint8_t u0 = *ptr++;
	if((u0 & 0x80) == 0) // 0xxxxxxx 
		return (uint32_t)u0;
	uint8_t u1 = *ptr++;
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	uint8_t u2 = *ptr++;
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	uint8_t u3 = *ptr++;
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC0) << 0;
	lxAssertNeverReach("Invalid utf8-codepoint");
}

size_t StringLengthUTF16(const char* str, size_t* outBytes)
{
	size_t length = 0;
	const char* base = str;
	while(AdvanceCursorUTF16(str))
		++length;
	if(outBytes)
		*outBytes = str - (const char*)base;
	return length;
}

u32 AdvanceCursorUTF16(const char*& _ptr)
{
	const u8* ptr = (const u8*)_ptr;
	u16 h = ((u16)ptr[0] | ((u16)ptr[1] << 8));
	if(h <= 0xD7FF || h >= 0xE000) {
		_ptr += 2;
		return h;
	}

	h &= ~0xFC00;
	u16 l = (ptr[2] | ptr[3] << 8);
	l &= ~0xFC00;

	u32 out = ((h << 10) | l) + 0x10000;

	_ptr += 4;
	return out;
}

u32 GetCharacterUTF16(const char* _ptr)
{
	const u8* ptr = (const u8*)_ptr;
	u16 h = ((u16)ptr[0] | ((u16)ptr[1] << 8));
	if(h <= 0xD7FF || h >= 0xE000) {
		_ptr += 2;
		return h;
	}

	h &= ~0xFC00;
	u16 l = (ptr[2] | ptr[3] << 8);
	l &= ~0xFC00;

	u32 out = ((h << 10) | l) + 0x10000;

	return out;
}

bool IsEqualCaseInsensitive(u32 a, u32 b)
{
	// TODO: This is wrong. Use case folding to do this
	return utf8proc_tolower(a) == utf8proc_tolower(b);
}

u32 ToLowerChar(u32 c)
{
	return utf8proc_tolower(c);
}

u32 ToUpperChar(u32 c)
{
	return utf8proc_toupper(c);
}

bool IsDigit(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_ND;
}

bool IsSpace(u32 c)
{
	auto cat = utf8proc_category(c);
	return c == '\n' || c == '\r' || c == ' ' || c == '\t' || cat == UTF8PROC_CATEGORY_ZS || cat == UTF8PROC_CATEGORY_ZL || cat == UTF8PROC_CATEGORY_ZP;
}

bool IsAlpha(u32 c)
{
	auto cat = utf8proc_category(c);
	return (cat == UTF8PROC_CATEGORY_LU ||
		cat == UTF8PROC_CATEGORY_LL ||
		cat == UTF8PROC_CATEGORY_LT ||
		cat == UTF8PROC_CATEGORY_LM ||
		cat == UTF8PROC_CATEGORY_LO);
}

bool IsUpper(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_LU;
}

bool IsLower(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_LL;
}

EUnicodeClass CategorizeCodePoint(u32 c)
{
	auto cat = utf8proc_category(c);
	switch(cat) {
	case UTF8PROC_CATEGORY_CN: return EUnicodeClass::Other;
	case UTF8PROC_CATEGORY_LU: return EUnicodeClass::LetterUpper;
	case UTF8PROC_CATEGORY_LL: return EUnicodeClass::LetterLower;
	case UTF8PROC_CATEGORY_LT: return EUnicodeClass::LetterTitel;
	case UTF8PROC_CATEGORY_LM: return EUnicodeClass::LetterModifier;
	case UTF8PROC_CATEGORY_LO: return EUnicodeClass::LetterOther;
	case UTF8PROC_CATEGORY_MN: return EUnicodeClass::MarkNonSpacing;
	case UTF8PROC_CATEGORY_MC: return EUnicodeClass::MarkSpacingCombining;
	case UTF8PROC_CATEGORY_ME: return EUnicodeClass::MarkEnclosing;
	case UTF8PROC_CATEGORY_ND: return EUnicodeClass::NumberDecimalDigit;
	case UTF8PROC_CATEGORY_NL: return EUnicodeClass::NumberLetter;
	case UTF8PROC_CATEGORY_NO: return EUnicodeClass::NumberOther;
	case UTF8PROC_CATEGORY_PC: return EUnicodeClass::PunctuationConnector;
	case UTF8PROC_CATEGORY_PD: return EUnicodeClass::PunctuationDash;
	case UTF8PROC_CATEGORY_PS: return EUnicodeClass::PunctuationOpen;
	case UTF8PROC_CATEGORY_PE: return EUnicodeClass::PunctuationClose;
	case UTF8PROC_CATEGORY_PI: return EUnicodeClass::PunctuationInitialQuote;
	case UTF8PROC_CATEGORY_PF: return EUnicodeClass::PunctuationFinalQuote;
	case UTF8PROC_CATEGORY_PO: return EUnicodeClass::PunctuationOther;
	case UTF8PROC_CATEGORY_SM: return EUnicodeClass::SymbolMath;
	case UTF8PROC_CATEGORY_SC: return EUnicodeClass::SymbolCurrency;
	case UTF8PROC_CATEGORY_SK: return EUnicodeClass::SymbolModifier;
	case UTF8PROC_CATEGORY_SO: return EUnicodeClass::SymbolOther;
	case UTF8PROC_CATEGORY_ZS: return EUnicodeClass::SeperatorSpace;
	case UTF8PROC_CATEGORY_ZL: return EUnicodeClass::SeperatorLine;
	case UTF8PROC_CATEGORY_ZP: return EUnicodeClass::SeperatorParagraph;
	case UTF8PROC_CATEGORY_CC: return EUnicodeClass::OtherControl;
	case UTF8PROC_CATEGORY_CF: return EUnicodeClass::OtherFormat;
	case UTF8PROC_CATEGORY_CS: return EUnicodeClass::OtherSurrogate;
	case UTF8PROC_CATEGORY_CO: return EUnicodeClass::OtherPrivateUse;
	default: lxAssertNeverReach("Unknown unicode category");
		return EUnicodeClass::Other;
	}
}

}
}