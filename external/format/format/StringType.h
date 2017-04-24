#pragma once
#include <cstdlib>

namespace format
{
enum StringType
{
	Ascii,   // 7-Bit ASCII
	Unicode, // UTF-8
	CodePoint// 32-Bit Unicode Codepoint
};

inline size_t GetBytePerChar(StringType t)
{
	switch(t) {
	case StringType::Unicode:
		return 0;
	case StringType::Ascii:
		return 1;
	case StringType::CodePoint:
		return 4;
	default:
		return 0;
	}
}
}