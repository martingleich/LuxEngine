#ifndef INCLUDED_LUX_IO_EXCEPTIONS_
#define INCLUDED_LUX_IO_EXCEPTIONS_
#include "core/lxException.h"

namespace lux
{
namespace io
{

//! A file couldn't be opend or doesn't exist.
struct FileNotFoundException : core::Exception
{
	explicit FileNotFoundException(const char* _path) :
		core::Exception(nullptr),
		path(_path)
	{
		m_What.Append("file_not_found(");
		m_What.Append(_path);
		m_What.Append(")");
	}

	//! The path to the invalid, only for debuging purposes.
	core::ExceptionSafeString path;
};

//! Exception while using a file.
struct FileException : core::Exception
{
	//! An error while writing to the file.
	static const int WriteError = 0;
	static const int ReadError = 1;
	static const int OutsideFile = 2;

	explicit FileException(int _type) :
		core::Exception("file error"),
		type(_type)
	{
	}

	//! The type of error which occured.
	int type;
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_LUX_IO_EXCEPTIONS_
