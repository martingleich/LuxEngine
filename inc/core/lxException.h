#ifndef INCLUDED_LX_EXCEPTION_H
#define INCLUDED_LX_EXCEPTION_H
#include "LuxBase.h"
#include <stdexcept>
#include <cstring>
#include <cstdlib>

namespace lux
{
namespace core
{

//! An exception safe string class
/**
This string class will _never_ throw an exception.
If the memory for the string can not be allocated, the string takes the value string_alloc_error.
*/
class ExceptionSafeString
{
public:
	ExceptionSafeString() :
		data("\001")
	{
	}

	ExceptionSafeString(const char* s)
	{
		size_t len = strlen(s);
		data = (char*)std::malloc(len + 1 + 1);
		if(!data)
			data = "\001string_alloc_error";
		else {
			data[0] = 1;
			memcpy(data + 1, s, len + 1);
		}
	}

	ExceptionSafeString(const ExceptionSafeString& other) :
		data(other.data)
	{
		++data[0];
	}

	ExceptionSafeString& operator=(const ExceptionSafeString& other)
	{
		this->~ExceptionSafeString();
		data = other.data;
		++data[0];

		return *this;
	}

	~ExceptionSafeString()
	{
		--data[0];
		if(!data[0])
			std::free(data);
	}

	const char* Data() const
	{
		return data + 1;
	}

	operator const char*() const
	{
		return Data();
	}

private:
	//! Contains a single byte reference count, followed by a null-terminated string.
	char* data;
};

struct Exception : public std::exception
{
	explicit Exception(const char* what) :
		m_What(what)
	{
	}

	const char* what() const
	{
		return m_What.Data();
	}

private:
	ExceptionSafeString m_What;
};

struct GenericException : Exception
{
	GenericException() :
		Exception("An error occured")
	{}
};


struct InvalidArgumentException : Exception
{
	explicit InvalidArgumentException(const char* _arg, const char* _assertion="") :
		Exception("invalid argument"),
		arg(_arg),
		assertion(_assertion)
	{
	}

	ExceptionSafeString arg;
	ExceptionSafeString assertion;
};

#define LX_CHECK_NULL_ARG(arg) if((arg) == nullptr) throw lux::core::InvalidArgumentException(#arg, "Must not be null");

struct FileNotFoundException : Exception
{
	explicit FileNotFoundException(const char* _path) :
		Exception("file not found"),
		path(_path)
	{
	}

	~FileNotFoundException()
	{
	}

	ExceptionSafeString path;
};

struct FileException : Exception
{
	static const int WriteError = 0;

	explicit FileException(int _type) :
		Exception("file error"),
		type(_type)
	{}


	int type;
};

struct UnicodeException : Exception
{
	explicit UnicodeException(const char* _msg, u32 _codepoint=0) :
		Exception("unicode exception"),
		msg(_msg),
		codepoint(_codepoint)
	{}

	ExceptionSafeString msg;
	u32 codepoint;
};

struct OutOfRangeException : Exception
{
	OutOfRangeException() :
		Exception("out of range")
	{}
};

struct ObjectNotFoundException : Exception
{
	explicit ObjectNotFoundException(const char* _object) :
		Exception("object not found"),
		object(_object)
	{}

	ExceptionSafeString object;
};

struct LoaderException : Exception
{
	LoaderException() :
		Exception("loader exception")
	{}
	explicit LoaderException(const char* msg) :
		Exception(msg)
	{}
};

struct FileFormatException : Exception
{
	explicit FileFormatException(const char* _format) :
		Exception("Fileformat not known or not supported"),
		format(_format)
	{}

	ExceptionSafeString format;
};

}
}

#endif // #ifndef INCLUDED_LX_EXCEPTION_H