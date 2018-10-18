#ifndef INCLUDED_LUX_EXCEPTION_H
#define INCLUDED_LUX_EXCEPTION_H
#include "LuxBase.h"
#include "core/lxStringView.h"
#include <cstring>
#include <cstdlib>
#include <stdarg.h>
#include <cstdio>

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
	struct Data
	{
		int refCount;
		int size;
		char bytes[1];
	};
private:
	static Data* Alloc(int size)
	{
		Data* out = (Data*)std::malloc(sizeof(Data) + size - 1);
		out->refCount = 1;
		out->size = size;
		return out;
	}

public:
	ExceptionSafeString() :
		ExceptionSafeString(core::StringView::EMPTY)
	{
	}

	ExceptionSafeString(core::StringView s)
	{
		m_Data = Alloc(s.Size());
		if(m_Data)
			std::memcpy(m_Data->bytes, s.Data(), s.Size());
	}
	
	ExceptionSafeString(const ExceptionSafeString& other) :
		m_Data(other.m_Data)
	{
		if(m_Data)
			++m_Data->refCount;
	}

	~ExceptionSafeString()
	{
		if(m_Data) {
			--m_Data->refCount;
			if(!m_Data->refCount)
				std::free(m_Data);
		}
	}

	ExceptionSafeString& operator=(const ExceptionSafeString& other)
	{
		this->~ExceptionSafeString();
		m_Data = other.m_Data;
		if(m_Data)
			++m_Data->refCount;

		return *this;
	}

	ExceptionSafeString& Append(core::StringView s)
	{
		if(s.IsEmpty() || !m_Data)
			return *this;

		int newlen = s.Size() + m_Data->size;
		Data* newdata = Alloc(newlen);
		if(newdata) {
			std::memcpy(newdata->bytes, m_Data->bytes, m_Data->size);
			std::memcpy(newdata->bytes + m_Data->size, s.Data(), s.Size());
		}
		this->~ExceptionSafeString();
		m_Data = newdata;
		return *this;
	}

	ExceptionSafeString& Append(const ExceptionSafeString& str)
	{
		return Append(str.AsView());
	}

	ExceptionSafeString& Append(int i)
	{
		char BUFFER[32];
		int len = snprintf(BUFFER, 32, "%d", i);
		return Append(StringView(BUFFER, len));
	}
	
	ExceptionSafeString& Append(float i)
	{
		char BUFFER[32];
		int len = snprintf(BUFFER, 32, "%f", i);
		return Append(StringView(BUFFER, len));
	}
	
	StringView AsView() const
	{
		if(!m_Data)
			return StringView("string_alloc_error");
		else
			return StringView(m_Data->bytes, m_Data->size);
	}
	
	ExceptionSafeString Copy() const { return ExceptionSafeString(AsView()); }

private:
	Data* m_Data;
};

//! The exception baseclass
struct Exception
{
	virtual ~Exception() {}
	virtual ExceptionSafeString What() const = 0;
};

struct InvalidCastException : public Exception
{
	virtual ExceptionSafeString What() const { return "InvalidCastException"; }
};

//! Exception which occured beyond the programmers control and most of the time can't be handled by him
/**
Examples are OutOfMemory, Errors when creating hardware resources, errors from os calls, etc.
These errors can't be handles by the programmer, except "this function call does not work."
*/
struct RuntimeException : Exception {};

struct GenericRuntimeException : RuntimeException
{
public:
	explicit GenericRuntimeException(core::StringView message) :
		m_Message(message)
	{}

	ExceptionSafeString What() const { return ExceptionSafeString("GenericRuntimeException: ").Append(m_Message); }

private:
	ExceptionSafeString m_Message;
};

struct FactoryCreateException : RuntimeException
{
	explicit FactoryCreateException(core::StringView name, core::StringView message) :
		m_Name(name),
		m_Message(message)
	{
	}

	ExceptionSafeString GetName() const { return m_Name; }
	ExceptionSafeString GetMessage() const { return m_Message; }
	ExceptionSafeString What() const { return ExceptionSafeString("FactoryCreateException: ").Append(m_Name).Append(": ").Append(m_Message); }

private:
	ExceptionSafeString m_Name;
	ExceptionSafeString m_Message;
};

//! Exception created by using a interface wrong
/**
Accessing arrays out of bound, calling unimplemented methods and so on.
*/
struct ErrorException : Exception { };

//! The requested thing is not implemented.
struct NotImplementedException : ErrorException
{
	explicit NotImplementedException(StringView name="") :
		m_Name(name)
	{
	}
	ExceptionSafeString What() const
	{
		return ExceptionSafeString("NotImplementedException: ").Append(m_Name);
	}
	ExceptionSafeString GetName() const { return m_Name; }
private:
	ExceptionSafeString m_Name;
};

//! Thrown if invalid arguments where based to a function
struct InvalidArgumentException : ErrorException
{
	explicit InvalidArgumentException(StringView _argName) :
		m_ArgName(_argName)
	{
	}
	ExceptionSafeString GetArgName() const { return m_ArgName; }
	ExceptionSafeString What() const
	{
		return ExceptionSafeString("InvalidArgumentException: ").Append(m_ArgName);
	}

private:
	ExceptionSafeString m_ArgName;
};

struct GenericInvalidArgumentException : InvalidArgumentException
{
	explicit GenericInvalidArgumentException(StringView _argName, StringView _message) :
		InvalidArgumentException(_argName),
		m_Message(_message)
	{
	}
	ExceptionSafeString GetMessage() const { return m_Message; }
	ExceptionSafeString What() const
	{
		return ExceptionSafeString("GenericInvalidArgumentException: ").Append(GetArgName()).Append(": ").Append(m_Message);
	}

private:
	ExceptionSafeString m_Message;
};

//! An array was accessed out of its valid range.
struct ArgumentOutOfRangeException : InvalidArgumentException
{
private:
	static ExceptionSafeString MakeMessage(ExceptionSafeString arg, int lo, int hi, int val)
	{
		return ExceptionSafeString("ArgumentOfOfRangeException: ").
			Append(arg).
			Append(" should be in [").
			Append(lo).
			Append(", ").
			Append(hi).
			Append(" but was ").
			Append(val);
	}
public:
	ArgumentOutOfRangeException(StringView argName, int begin, int end, int val) :
		InvalidArgumentException(argName),
		m_Lo(begin),
		m_Hi(end),
		m_Val(val)
	{
	}
	ExceptionSafeString What() const { return MakeMessage(GetArgName(), m_Lo, m_Hi, m_Val); }

private:
	int m_Lo;
	int m_Hi;
	int m_Val;
};

struct FloatArgumentOutOfRangeException : InvalidArgumentException
{
private:
	static ExceptionSafeString MakeMessage(ExceptionSafeString arg, float lo, float hi, float val)
	{
		return ExceptionSafeString("FloatArgumentOfOfRangeException: ").
			Append(arg).
			Append(" should be in [").
			Append(lo).
			Append(", ").
			Append(hi).
			Append(" but was ").
			Append(val);
	}
public:
	FloatArgumentOutOfRangeException(StringView argName, float lo, float hi, float val) :
		InvalidArgumentException(argName),
		m_Lo(lo),
		m_Hi(hi),
		m_Val(val)
	{
	}
	ExceptionSafeString What() const { return MakeMessage(GetArgName(), m_Lo, m_Hi, m_Val); }

private:
	float m_Lo;
	float m_Hi;
	float m_Val;
};

struct ArgumentNullException : InvalidArgumentException
{
private:
	static ExceptionSafeString MakeMessage(ExceptionSafeString arg)
	{
		return ExceptionSafeString("ArgumentNullException: ").Append(arg);
	}
public:
	explicit ArgumentNullException(StringView argName) :
		InvalidArgumentException(argName)
	{
	}
	ExceptionSafeString What() const { return MakeMessage(GetArgName()); }
};

//! Helper macro to check for not allowed NULL-Pointer arguments.
#define LX_CHECK_NULL_ARG(arg) if(!(arg)) throw lux::core::ArgumentNullException(#arg);

//! Each argument will be evaluated multiple times.
#define LX_CHECK_BOUNDS(arg, lo, hi) if((arg) < (lo) || (arg) >= (hi)) throw lux::core::ArgumentOutOfRangeException(#arg, (lo), (hi), (arg));

struct InvalidOperationException : public ErrorException
{
	explicit InvalidOperationException(StringView operation) :
		m_Operation(operation)
	{
	}

	ExceptionSafeString GetOperation() const { return m_Operation; }
	ExceptionSafeString What() const { return ExceptionSafeString("InvalidOperationException: ").Append(m_Operation); }

private:
	ExceptionSafeString m_Operation;
};

//! Exception while dealing with unicode strings or characters.
struct UnicodeException : Exception
{
	explicit UnicodeException(u32 _codepoint) :
		codepoint(_codepoint)
	{
	}

	u32 GetCodePoint() const { return codepoint; }
	ExceptionSafeString What() const { return "UnicodeException"; }
private:
	u32 codepoint;
};

//! A non existing object was queried.
/**
For example a parameter in a ParamPackage
*/
struct ObjectNotFoundException : ErrorException
{
	explicit ObjectNotFoundException(StringView _object) :
		m_Object(_object)
	{
	}

	ExceptionSafeString GetObjectName() const { return m_Object; }
	ExceptionSafeString What() const { return ExceptionSafeString("ObjectNotFoundException: ").Append(m_Object); }
private:
	ExceptionSafeString m_Object;
};

struct ObjectAlreadyExistsException : ErrorException
{
	explicit ObjectAlreadyExistsException(StringView _object) :
		m_Object(_object)
	{
	}

	ExceptionSafeString GetObjectName() const { return m_Object; }
	ExceptionSafeString What() const { return ExceptionSafeString("ObjectAlreadyExists: ").Append(m_Object); }
private:
	ExceptionSafeString m_Object;
};

//! Exception dealing with a file format
/**
Thrown when a fileformat is not available or the content is corrupted.
*/
struct FileFormatException : Exception
{
	explicit FileFormatException(StringView _msg, StringView _format) :
		format(_format),
		msg(_msg)
	{
	}

	ExceptionSafeString GetFileFormat() const { return format; }
	ExceptionSafeString GetMessage() const { return msg; }
	ExceptionSafeString What() const { return ExceptionSafeString("FileFormatException: ").Append(format).Append(" :").Append(msg); }
private:
	ExceptionSafeString format;
	ExceptionSafeString msg;
};

} // namespace core
} // namespace lux 

#endif // #ifndef INCLUDED_LUX_EXCEPTION_H
