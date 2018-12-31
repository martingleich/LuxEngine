#pragma once
#include "FormatConfig.h"
#include "FormatMemoryFwd.h"
#include <cassert>

namespace format
{
namespace parser
{
struct SaveStringReader
{
	const char* begin;
	const char* str;
	const char* end;

	explicit SaveStringReader(Slice s) :
		begin(s.data),
		str(s.data),
		end(s.data+s.size)
	{}
	char Peek() const
	{
		assert(str != end);
		return *str;
	}
	char Get()
	{
		assert(str != end);
		return *str++;
	}
	bool CheckAndGet(char c)
	{
		if(IsEnd())
			return false;
		if(Peek() == c) {
			Get();
			return true;
		}
		return false;
	}
	bool IsEnd() const { return str == end; }
};

FORMAT_API bool TryReadInteger(SaveStringReader& reader, int& out);
FORMAT_API void SkipSpace(SaveStringReader& reader);

FORMAT_API Slice ReadWord(SaveStringReader& reader);

}
}

