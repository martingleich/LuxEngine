#ifndef INCLUDED_LX_EXCEPTION_H
#define INCLUDED_LX_EXCEPTION_H
#include "LuxBase.h"
#include <cstring>
#include <cstdlib>
#include <stdarg.h>

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
		ExceptionSafeString("")
	{
	}

	ExceptionSafeString(const char* s)
	{
		if(!s)
			s = "";

		size_t len = strlen(s);
		data = (char*)std::malloc(len + 1 + 1);
		if(data) {
			data[0] = 1;
			memcpy(data + 1, s, len + 1);
		}
	}

	ExceptionSafeString& Append(const char* s)
	{
		if(!s)
			return *this;

		if(!data) {
			*this = s;
			return *this;
		}


		size_t oldlen = strlen(Data());
		size_t newlen = strlen(s) + oldlen;
		char* newdata = (char*)std::malloc(newlen + 1 + 1);
		if(newdata) {
			newdata[0] = 1;
			memcpy(newdata + 1, Data(), oldlen);
			memcpy(newdata + 1 + oldlen, s, newlen - oldlen + 1);
		}

		this->~ExceptionSafeString();
		data = newdata;

		return *this;
	}

	ExceptionSafeString(const ExceptionSafeString& other) :
		data(other.data)
	{
		if(data)
			++data[0];
	}

	ExceptionSafeString& operator=(const ExceptionSafeString& other)
	{
		this->~ExceptionSafeString();
		data = other.data;
		if(data)
			++data[0];

		return *this;
	}

	~ExceptionSafeString()
	{
		if(data) {
			--data[0];
			if(!data[0])
				std::free(data);
		}
	}

	const char* Data() const
	{
		if(data)
			return data + 1;
		else
			return "string_alloc_error";
	}

	operator const char*() const
	{
		return Data();
	}

private:
	//! Contains a single byte reference count, followed by a null-terminated string.
	char* data;
};

//! The exception baseclass
struct Exception
{
	explicit Exception(const char* what) :
		m_What(what)
	{
	}

	const char* What() const noexcept
	{
		return m_What.Data();
	}

protected:
	ExceptionSafeString m_What;
};

struct InvalidCastException : public Exception
{
	InvalidCastException() :
		Exception("Invalid cast")
	{}
};

//! Exception which occured beyond the programmers control and most of the time can't be handled by him
/**
Examples are OutOfMemory, Errors when creating hardware resources, errors from os calls, etc.
These errors can't be handles by the programmer, except "this function call does not work."
*/
struct RuntimeException : Exception
{
	explicit RuntimeException(const char* msg) :
		Exception(msg)
	{
	}
};

//! Exception created by using a interface wrong
/**
Accessing arrays out of bound, calling unimplemented methods and so on.
*/
struct ErrorException : Exception
{
	explicit ErrorException(const char* msg) :
		Exception(msg)
	{
	}
};

//! The requested thing is not implemented.
struct NotImplementedException : ErrorException
{
	NotImplementedException() :
		ErrorException("not implemented")
	{
	}
};

//! Thrown if invalid arguments where based to a function
struct InvalidArgumentException : ErrorException
{
	explicit InvalidArgumentException(const char* _arg, const char* _assertion = "") :
		ErrorException(nullptr),
		arg(_arg),
		assertion(_assertion)
	{
		m_What.Append("invalid_argument(");
		m_What.Append(arg);
		if(assertion) {
			m_What.Append(", ");
			m_What.Append(assertion);
		}
		m_What.Append(")");
	}

	//! The argument which was invalid
	ExceptionSafeString arg;
	//! Why was the argument invalid
	ExceptionSafeString assertion;
};

//! Helper macro to check for not allowed NULL-Pointer arguments.
#define LX_CHECK_NULL_ARG(arg) if(!(arg)) throw lux::core::InvalidArgumentException(#arg, "Must not be null");

#define LX_CHECK_BOUNDS(arg, lo, hi) if(arg < lo || arg > hi) throw lux::core::InvalidArgumentException(#arg, "Is out of range.");

//! A file couldn't be opend or doesn't exist.
struct FileNotFoundException : Exception
{
	explicit FileNotFoundException(const char* _path) :
		Exception(nullptr),
		path(_path)
	{
		m_What.Append("file_not_found(");
		m_What.Append(_path);
		m_What.Append(")");
	}

	//! The path to the invalid, only for debuging purposes.
	ExceptionSafeString path;
};

//! Exception while using a file.
struct FileException : Exception
{
	//! An error while writing to the file.
	static const int WriteError = 0;
	static const int ReadError = 1;
	static const int OutsideFile = 2;

	explicit FileException(int _type) :
		Exception("file error"),
		type(_type)
	{
	}

	//! The type of error which occured.
	int type;
};

//! Exception while dealing with unicode strings or characters.
struct UnicodeException : Exception
{
	explicit UnicodeException(const char* _msg, u32 _codepoint = 0) :
		Exception("unicode exception"),
		msg(_msg),
		codepoint(_codepoint)
	{
	}

	ExceptionSafeString msg;

	//! The codepoint which generated the error, may be 0
	u32 codepoint;
};

//! An array was accessed out of its valid range.
struct OutOfRangeException : ErrorException
{
	OutOfRangeException() :
		ErrorException("out of range")
	{
	}
};

//! A non existing object was queried.
/**
For example a parameter in a ParamPackage
*/
struct ObjectNotFoundException : ErrorException
{
	explicit ObjectNotFoundException(const char* _object) :
		ErrorException(nullptr),
		object(_object)
	{
		m_What.Append("object_not_found(");
		m_What.Append(object);
		m_What.Append(")");
	}

	//! The name of the queried object.
	ExceptionSafeString object;
};

//! Exception dealing with a file format
/**
Thrown when a fileformat is not available or the contet is corrupted.
*/
struct FileFormatException : Exception
{
	explicit FileFormatException(const char* _msg, const char* _format = "") :
		Exception("Fileformat not known or not supported"),
		format(_format),
		msg(_msg)
	{
	}

	//! The name of the problematic format, may be empty
	ExceptionSafeString format;
	ExceptionSafeString msg;
};

} // namespace core
} // namespace lux 

#endif // #ifndef INCLUDED_LX_EXCEPTION_H