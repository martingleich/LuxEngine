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
		m_Path(_path)
	{
	}

	core::ExceptionSafeString GetPath() const { return m_Path; }
	core::ExceptionSafeString What() const { return core::ExceptionSafeString("FileNotFoundException: ").Append(m_Path); }
private:
	core::ExceptionSafeString m_Path;
};

//! Exception while using a file.
struct FileUsageException : core::Exception
{
	//! An error while writing to the file.
	enum EError
	{
		WriteError,
		ReadError,
		CursorOutsideFile,
	};

	explicit FileUsageException(EError error, const core::String& path) :
		m_Error(error),
		m_Path(path.Data())
	{
	}
	explicit FileUsageException(EError error, const char* path) :
		m_Error(error),
		m_Path(path)
	{
	}
	EError GetError() const { return m_Error; }
	core::ExceptionSafeString GetPath() const { return m_Path; }
	core::ExceptionSafeString What() const {
		core::ExceptionSafeString out("FileUsageException: ");
		switch(m_Error) {
		case WriteError: out.Append("WriteError "); break;
		case ReadError: out.Append("ReadError "); break;
		case CursorOutsideFile: out.Append("CursorOutsideFile "); break;
		default: out.Append("UnknownError "); break;
		}
		out.Append(m_Path);
		return out;
	}

private:
	EError m_Error;
	core::ExceptionSafeString m_Path;
};

} // namespace io
} // namespace lux

#endif // #ifndef INCLUDED_LUX_IO_EXCEPTIONS_
