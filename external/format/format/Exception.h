#pragma once
#include <cassert>
#include <cstdlib>

namespace format
{
struct exception
{
	exception(const char* _msg = "format_exception") :
		msg(_msg)
	{
	}

	const char* msg; // Always ascii
};

struct syntax_exception : public exception
{
	syntax_exception(const char* msg = "format_syntax_exception", size_t pos = -1) :
		exception(msg),
		position(pos)
	{
	}

	size_t position;
};

struct invalid_placeholder_type : public syntax_exception
{
	invalid_placeholder_type(const char* msg = "invalid_placeholder_type", size_t pos = -1, size_t argPos=-1, char place=0) :
		syntax_exception(msg, pos),
		argPosition(argPos),
		placeholder(place)
	{

	}

	size_t argPosition;
	char placeholder;
};

struct invalid_placeholder_value : public syntax_exception
{
	invalid_placeholder_value(const char* msg = "invalid_placeholder_type", size_t pos = -1, int v=0) :
		syntax_exception(msg, pos),
		value(v)
	{

	}

	int value;
};

struct value_exception : public exception
{
	value_exception(const char* msg = "format_value_exception", size_t pos=-1, size_t argPos=-1) :
		exception(msg),
		position(pos),
		argPosition(argPos)
	{
	}

	size_t position;
	size_t argPosition;
};

struct not_implemeted_exception : public exception
{
	not_implemeted_exception(const char* msg = "format_not_implemented_exception") :
		exception(msg)
	{
	}
};

struct bad_cast_exception : public exception
{
	bad_cast_exception(const char* msg = "format_bad_cast_exception") :
		exception(msg)
	{
	}
};

struct bad_locale_exception : public exception
{
	bad_locale_exception(const char* msg = "bad_locale_exception") :
		exception(msg)
	{
	}
};

}