#ifndef INCLUDED_FORMAT_EXCEPTION_H
#define INCLUDED_FORMAT_EXCEPTION_H
#include <cstddef>
#include <exception>

namespace format
{
/** \addtogroup Exceptions
@{
*/
struct format_exception : std::exception
{
	format_exception(const char* msg = "format_exception") :
		exception(msg)
	{
	}
};

struct syntax_exception : public format_exception
{
	syntax_exception(const char* msg = "format_syntax_exception", size_t pos = -1) :
		format_exception(msg),
		position(pos)
	{
	}

	size_t position;
};

struct invalid_placeholder_type : public syntax_exception
{
	invalid_placeholder_type(const char* msg = "invalid_placeholder_type", size_t pos = -1, size_t argPos = -1, char place = 0) :
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
	invalid_placeholder_value(const char* msg = "invalid_placeholder_type", size_t pos = -1, int v = 0) :
		syntax_exception(msg, pos),
		value(v)
	{
	}

	int value;
};

struct value_exception : public format_exception
{
	value_exception(const char* msg = "format_value_exception", size_t pos = -1, size_t argPos = -1) :
		format_exception(msg),
		position(pos),
		argPosition(argPos)
	{
	}

	size_t position;
	size_t argPosition;
};

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_EXCEPTION_H
