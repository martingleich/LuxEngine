#ifndef INCLUDED_LUX_ARCHIVE_H
#define INCLUDED_LUX_ARCHIVE_H
#include "core/ReferenceCounted.h"
#include "io/ioConstants.h"
#include "io/Path.h"

namespace lux
{
namespace io
{
class Archive;

//! A file enumerator
/**
File enumerators iterate over some list of files.
After the enumerator is placed before the first file, i.e. Before accesing files a call to Advance is requiered
*/
class AbstractFileEnumerator : public ReferenceCounted
{
public:
	virtual ~AbstractFileEnumerator() {}

	//! Advance the enumerator to the next file.
	/**
	\return Was the enumerator advanced correctly
	*/
	virtual bool Advance() = 0;

	virtual const FileInfo& GetInfo() const = 0;
	virtual const Path& GetBasePath() const = 0;
	virtual const core::String& GetName() const = 0;
	virtual const Path& GetFullPath() const = 0;
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
	virtual StrongRef<File> OpenFile(const Path& p, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false) = 0;

	//! Check if a file exists
	virtual bool ExistFile(const Path& p) const = 0;

	//! Check if a directory exists.
	virtual bool ExistDirectory(const Path& p) const = 0;

	//! Generate information for a file.
	virtual FileInfo GetFileInfo(const Path& p) const = 0;

	//! Create a enumerator for a subdirecorty in the archive.
	virtual StrongRef<AbstractFileEnumerator> EnumerateFiles(const Path& subDir = io::Path::EMPTY) = 0;

	//! Get the capabilites of the archive
	virtual EArchiveCapFlag GetCaps() const = 0;

	//! Convert a path inside the archive to an absolute path.
	/**
	This only works if the requested file has an absolute path in the _real_ file system.
	*/
	virtual Path GetAbsolutePath(const Path& p) const = 0;

	//! Get the path of the archive, may be empty.
	virtual const Path& GetPath() const = 0;

	virtual void CreateFile(const Path& path, bool recursive) = 0;
	virtual void DeleteFile(const Path& path) = 0;
	virtual void CreateDirectory(const Path& path, bool recursive) = 0;
	virtual void DeleteDirectory(const Path& path) = 0;
};

}
}

#endif // #ifndef INCLUDED_LUX_ARCHIVE_H
