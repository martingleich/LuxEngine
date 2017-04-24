#ifndef INCLUDED_FILE_H
#define INCLUDED_FILE_H
#include "core/ReferenceCounted.h"
#include "math/lxMath.h"
#include "io/path.h"
#include "ioConstants.h"

namespace lux
{
namespace io
{

//! A file usable for reading and writing
class File : public ReferenceCounted
{
protected:
	string m_Name;
	FileDescription m_Desc;

public:
	File(const string& name, const FileDescription& desc) : m_Name(name), m_Desc(desc)
	{
	}
	virtual ~File()
	{
	}

	//! Get the name of the file
	/**
	May be an empty string, must not be the path of the file
	\return The name of the file
	*/
	virtual const string& GetName() const
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
	\param Length The number of bytes to write
	\return The number of bytes which were written, 0 in the case of an error
	*/
	virtual u32 WriteBinary(const void* data, u32 length) = 0;

	//! Write a string to the file
	/**
	\return The number of bytes which were written, at max str.Size()+1
	*/
	virtual u32 WriteString(const string& str)
	{
		lxAssert(str.Size() < std::numeric_limits<u32>::max());
		return WriteBinary(str.Data(), (u32)str.Size());
	}

	//! Write the content of a virtual file to this file
	/**
	\param file The file to copy
	\param bytes the number of bytes to copy, or 0 to copy bytes until end of file
	\return The number of bytes written, in the case of an error 0
	*/
	u32 WriteToFile(File* file, u32 bytes = 0)
	{
		static u8 byteBuffer[1024];
		u32 written = 0;
		if(bytes > 0) {
			if(file->GetBuffer() != nullptr) {
				written = this->WriteBinary(file->GetBuffer(), file->GetSize());
			} else {
				while(written < bytes) {
					u32 toMove = math::Min((u32)sizeof(byteBuffer), (u32)bytes);
					u32 readBytes = file->ReadBinary(toMove, byteBuffer);
					u32 writtenBytes = this->WriteBinary(byteBuffer, readBytes);
					written += writtenBytes;
					if(writtenBytes != toMove)
						break;
				}
			}
		} else {
			while(!file->IsEOF()) {
				u32 toMove = sizeof(byteBuffer);
				u32 readBytes = file->ReadBinary(toMove, byteBuffer);
				u32 writtenBytes = this->WriteBinary(byteBuffer, readBytes);
				written += writtenBytes;
				if(writtenBytes != toMove)
					break;
			}
		}

		return written;
	}

	//! Read binary data
	/**
	Read bytes from the cursor to the end of the file
	\param numBytes The number of bytes to read
	\param out Here the number of bytes are written
	\return The number of read bytes, in the case of an error 0
	*/
	virtual u32 ReadBinary(u32 numBytes, void* out) = 0;

	//! Read any type
	/**
	The type is read by its binary representation
	should only be used for POD types, with fixes alignment
	\return The read value, Binary 0 if it couldnt read
	*/
	template <typename type>
	type Read()
	{
		type out(0);
		ReadBinary(sizeof(type), &out);
		return out;
	}

	template <>
	string Read()
	{
		return ReadString(0);
	}

	// Liest einen str ein, hört auf wenn eine '\0' gelesen wird, oder die maximale Zeichenanzahl gelesen wurde
	//! Read a string from the file
	/**
	Read single utf-8 bytes from file until 0 is encounterd
	or the maximal number of bytes is reached
	\param size The maximal number of bytes to read
	\return The read string
	*/
	virtual string ReadString(u32 size = 0)
	{
		string str;
		if(size != 0)
			str.Reserve(size);
		else
			str.Reserve(256);

		u32 bytesRead = 0;
		while(true) {
			u8 c = Read<u8>();
			if(c == 0)
				break;

			str.PushByte(c);
			++bytesRead;

			if(bytesRead == size)
				break;
		}

		return str;
	}

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
	\return Could the cursor moved to this position
	*/
	virtual bool Seek(s32 offset, ESeekOrigin orgin = ESeekOrigin::Cursor) = 0;

	//! The size of the file in bytes
	virtual u32 GetSize() const = 0;

	//! The current cursor position in the file
	virtual u32 GetCursor() const = 0;
};

}    //namespace io
}    //namespace lux

#endif