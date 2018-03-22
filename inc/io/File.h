#ifndef INCLUDED_FILE_H
#define INCLUDED_FILE_H
#include "core/ReferenceCounted.h"
#include "math/lxMath.h"
#include "io/Path.h"
#include "io/ioConstants.h"
#include "io/ioExceptions.h"

namespace lux
{
namespace io
{
//! A file usable for reading and writing
class File : public ReferenceCounted
{
public:
	File(const core::String& name, const FileDescription& desc) : m_Name(name), m_Desc(desc)
	{
	}
	virtual ~File() {}

	//! Get the name of the file
	/**
	May be an empty string, must not be the path of the file
	\return The name of the file
	*/
	virtual const core::String& GetName() const
	{
		return m_Name;
	}

	//! Get a full description of the file
	/**
	Including size, path, source archive etc.
	\return The description of the file
	*/
	virtual const FileDescription& GetDescription() const
	{
		return m_Desc;
	}

	//! Is the filecursor at the end of the file
	virtual bool IsEOF() const
	{
		return GetCursor() == GetSize();
	}

	//! Write binary data to the file
	/**
	\param data The data to write to the file
	\param numBytes The number of bytes to write
	*/
	void WriteBinary(const void* data, s64 numBytes)
	{
		s64 count = WriteBinaryPart(data, numBytes);
		if(count != numBytes)
			throw io::FileException(io::FileException::WriteError);
	}

	//! Write binary data to the file
	/**
	\param data The data to write to the file
	\param numBytes The number of bytes to write
	\return The number of bytes writtefn, may be smaller than numBytes
	*/
	virtual s64 WriteBinaryPart(const void* data, s64 numBytes) = 0;

	//! Read binary data
	/**
	Read bytes from the cursor
	\param numBytes The number of bytes to read
	\param out Here the number of bytes are written
	*/
	void ReadBinary(s64 numBytes, void* out)
	{
		s64 count = ReadBinaryPart(numBytes, out);
		if(count != numBytes)
			throw io::FileException(io::FileException::ReadError);
	}

	//! Read binary data
	/**
	Read bytes from the cursor
	\param numBytes The number of bytes to read
	\param out Here the number of bytes are written
	\return The number of bytes read, may be smaller than numBytes
	*/
	virtual s64 ReadBinaryPart(s64 numBytes, void* out) = 0;

	//! A binary buffer of the file data
	/**
	The buffer begins at the begin at the file
	and is at least File::GetSize bytes long
	\return The binary data of the file, or NULL if no such buffer exists
	*/
	virtual const void* GetBuffer() const = 0;

	//! A binary buffer of the file data
	/**
	The buffer begins at the begin at the file
	and is at least File::GetSize bytes long
	\return The binary data of the file, or NULL if no such buffer exists
	*/
	virtual void* GetBuffer() = 0;

	//! Move the file cursor
	/**
	If the cursor could be moved to the correct position
	it isn't moved at all
	\param Origin The origin of the seek operation
	\param offset The seek offset
	*/
	virtual void Seek(s64 offset, ESeekOrigin origin = ESeekOrigin::Cursor) = 0;

	//! The size of the file in bytes
	virtual s64 GetSize() const = 0;

	//! The current cursor position in the file
	virtual s64 GetCursor() const = 0;

protected:
	core::String m_Name;
	FileDescription m_Desc;
};

}    //namespace io
}    //namespace lux

#endif
