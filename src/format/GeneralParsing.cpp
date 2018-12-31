#include "format/GeneralParsing.h"
#include "format/Exception.h"
#include <climits>

namespace format
{
namespace parser
{
static bool IsDigit(char c) { return c >= '0' && c <= '9'; }

static int CharToInt(char c) { return c - '0'; }

bool TryReadInteger(SaveStringReader& reader, int& out)
{
	if(reader.IsEnd() || !IsDigit(reader.Peek()))
		return false;

	out = 0;
	while(!reader.IsEnd() && IsDigit(reader.Peek())) {
		if(out > (INT_MAX - CharToInt(reader.Peek())) / 10)
			throw syntax_exception("Integer literal is too big.", (size_t)-1);
		out *= 10;
		out += CharToInt(reader.Peek());
		reader.Get();
	}

	return true;
}

void SkipSpace(SaveStringReader& reader)
{
	while(!reader.IsEnd() && reader.Peek() == ' ')
		reader.Get();
}

static bool IsIdentStartChar(char c)
{
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}
static bool IsIdentRestChar(char c)
{
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') ||
		c == '_';
}

Slice ReadWord(SaveStringReader& reader)
{
	if(!IsIdentStartChar(reader.Peek()))
		return Slice();
	const char* start = reader.str;
	reader.Get();
	while(IsIdentRestChar(reader.Peek()))
		reader.Get();
	return Slice(int(reader.str - start), start);
}
}
}
