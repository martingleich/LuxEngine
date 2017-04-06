#ifndef INCLUDED_LXCHARACTER
#define INCLUDED_LXCHARACTER
#include <cwctype>

namespace lux
{
namespace core
{

//! Convert to lower-case
inline char ToLowerChar(char c)
{
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

inline u32 ToLowerChar(u32 c)
{
	return (u32)std::towlower((wint_t)c);
}

//! Convert to upper-case
inline char ToUpperChar(char c)
{
	return (c >= 'a' && c <= 'z') ? c + ('A' - 'a') : c;
}

inline u32 ToUpperChar(u32 c)
{
	return (u32)std::towupper((wint_t)c);
}

//! Is the character a digit
inline bool IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool IsDigit(u32 c)
{
	return std::iswdigit((wint_t)c) != 0;
}

//! Is the character invisible if printed
inline bool IsSpace(char c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

inline bool IsSpace(u32 c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

//! Is the character a letter
inline bool IsAlpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool IsAlpha(u32 c)
{
	return (std::iswalpha((wint_t)c) != 0);
}

//! Is the character in upper-case
inline bool IsUpper(char c)
{
	return c >= 'A' && c <= 'Z';
}

inline bool IsUpper(u32 c)
{
	return (std::iswupper((wint_t)c) != 0);
}

//! Is the character in lower-case
inline bool IsLower(char c)
{
	return c >= 'a' && c <= 'z';
}

inline bool IsLower(u32 c)
{
	return (std::iswlower((wint_t)c) != 0);
}

}
}

#endif // #ifndef INCLUDED_LXCHARACTER
