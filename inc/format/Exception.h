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
	syntax_exception(const char* msg, size_t pos) :
		format_exception(msg),
		position(pos)
	{
	}

	size_t position;
};

struct invalid_placeholder_type : public syntax_exception
{
	invalid_placeholder_type(const char* msg, size_t pos) :
		syntax_exception(msg, pos)
	{
	}
};

struct invalid_placeholder_value : public syntax_exception
{
	invalid_placeholder_value(const char* msg, size_t pos, int v) :
		syntax_exception(msg, pos),
		value(v)
	{
	}

	int value;
};

struct invalid_argument : public format_exception
{
	invalid_argument(const char* msg, size_t _argId) :
		format_exception(msg),
		argId(_argId)
	{
	}

	size_t argId;
};

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_EXCEPTION_H
