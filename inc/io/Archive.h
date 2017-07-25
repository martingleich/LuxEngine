#ifndef INCLUDED_ARCHIVE_H
#define INCLUDED_ARCHIVE_H
#include "core/ReferenceCounted.h"
#include "io/ioConstants.h"
#include "io/path.h"
#include "core/lxIterator.h"

namespace lux
{
namespace io
{
class File;
class Archive;

//! A file enumerator
/**
File enumerators iterate over some list of files.
The list and order depends on the file enumerator i.e. The way it was created.
*/
class FileEnumerator : public ReferenceCounted
{
public:
	virtual ~FileEnumerator() {}

	//! Points the enumerator to a valid file.
	/**
	If advanced over the last file this will always be false.
	*/
	virtual bool IsValid() const = 0;

	//! Advance the enumerator to the next file.
	/**
	\return Is the file pointed after the Advance valid, i.e. the return value of IsValid
	*/
	virtual bool Advance() = 0;

	//! The the description of the current file.
	virtual const FileDescription& GetCurrent() const = 0;
};

//! A file archive
/**
Any collection of files and directories can be seen as archive, i.e. a _real_ folder, a zip file etc.
*/
class Archive : public ReferenceCounted
{
public:
	//! Open a file inside the archive.
	/**
	\param p The path inside the archive to the file
	\param mode The mode to open the file in.
	\param createIfNotExist If the file doesn't already exists, should it be created.
	\return The opend file
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenFile(const Path& p, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;

	//! Open a file inside the archive.
	/**
	\param file The file descriptor of the file, must reference this archive.
	\param mode The mode to open the file in.
	\param createIfNotExist If the file doesn't already exists, should it be created.
	\return The opend file
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenFile(const FileDescription& file, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;

	//! Check if a files exists
	virtual bool ExistFile(const Path& p) = 0;

	//! Create a enumerator for a subdirecorty in the archive.
	virtual StrongRef<FileEnumerator> EnumerateFiles(const Path& subDir = String::EMPTY) = 0;

	//! Get the capabilites of the archive
	virtual EArchiveCapabilities GetCaps() const = 0;

	//! Convert a path inside the archive to an absolute path.
	/**
	This only works if the requested file has an absolute path in the _real_ file system.
	*/
	virtual Path GetAbsolutePath(const Path& p) = 0;
};

}
}
#endif // #ifndef INCLUDED_ARCHIVE_H
