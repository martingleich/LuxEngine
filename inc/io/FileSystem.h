#ifndef INCLUDED_FILESYSTEM_H
#define INCLUDED_FILESYSTEM_H
#include "core/ReferenceCounted.h"

#include "core/DateAndTime.h"
#include "io/ioConstants.h"
#include "io/Path.h"

namespace lux
{
namespace io
{

class File;
class INIFile;
class Archive;

//! The lux filesystem
/**
Create new files form disc or memory
Write files to disc
Get information, or do anything about files
*/
class FileSystem : public ReferenceCounted
{
public:
	virtual ~FileSystem()
	{
	}

	//! Initialize the global filesystem
	LUX_API static void Initialize(FileSystem* fileSys = nullptr);

	//! Access the global filesystem
	LUX_API static FileSystem* Instance();

	//! Destroys the global filesystem
	LUX_API static void Destroy();

	//! Open a new file from file description
	/**
	Will only open non virtual files.
	\param desc The file description to open
	\param mode The mode to open the file in
	\param createIfNotExist If the file doesnt exist create it
	\return The newly created file
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenFile(const FileDescription& desc, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;

	//! Open a new file
	/**
	\param filename The name of the file to open
	\param mode The mode to open the file in
	\param createIfNotExist If the file doesnt exist create it
	\return The newly created file
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenFile(const Path& filename, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;

	//! Create a file from Memory for read and write
	/**
	\param memory The memory location to use for the file, dont delete this while the file is in usage
	\param size The size of the memory used by the file in bytes
	\param name The name of the newly created file, can be empty
	\param flags Flags for the creation of a virtual file.
	\return The newly created file.
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenVirtualFile(void* memory, s64 size, const core::String& name, EVirtualCreateFlag flags = EVirtualCreateFlag::None) = 0;
	virtual StrongRef<File> OpenVirtualFile(const void* memory, s64 size, const core::String& name, EVirtualCreateFlag flags = EVirtualCreateFlag::None) = 0;

	//! Create a limited file
	/**
	A limited file is a file which represents an part on an larger file
	\param file The file of which the limited file is part of.
	\param start The begin of the limited file in bytes.
	\param size The size of the limited file in bytes.
	\param name The name of the new file
	\return The new file
	\throws FileNotFoundException
	*/
	virtual StrongRef<File> OpenLimitedFile(File* file, s64 start, s64 size, const core::String& name) = 0;

	//! Test if a file exist
	/**
	\param path The filename to test
	\return True, if the file exists otherwise false
	*/
	virtual bool ExistFile(const Path& path) const = 0;

	//! The if a directory exists
	/**
	\param path The filename to test
	\return True if the directory exists otherwise false
	*/
	virtual bool ExistDirectory(const Path& path) const = 0;

	//! Expands an filename to an absolute path
	/**
	\param filename The filename to use
	\return The absolute filename of this file
	*/
	virtual Path GetAbsoluteFilename(const Path& filename) const = 0;

	//! Get the current working directory
	/**
	All relative Pathes, orgins from this path
	\return The current relative path
	*/
	virtual const Path& GetWorkingDirectory() const = 0;

	//! Create a new temporary file
	/**
	A temporary file, will be removed from disc after its no longer referenzed
	\param size The roundabout size of the new file, didnt have to be exakt
	\return The newly created file.
	\throws FileNotFoundException
	*/
	virtual File* CreateTemporaryFile(s64 size) = 0;

	//! Retrieve important fileinformations
	/**
	\param path The path of the file or directory
	\return The information about the file is written here
	\throws FileNotFoundException
	*/
	virtual FileDescription GetFileDescription(const Path& path) = 0;

	//! Create a ini file reader
	/**
	\param filename The path of the ini file
	\return The new INI File
	\throws FileNotFoundException
	*/
	virtual StrongRef<INIFile> CreateINIFile(const Path& filename) = 0;

	//! Create a ini file reader
	/**
	\param file The file to open
	\return The new INI File
	*/
	virtual StrongRef<INIFile> CreateINIFile(File* file) = 0;

	//! Create a file or directory
	/**
	\param path The path of the new file if the file ends with a / its a directory
	\param recursive If this parameter is true, and the subpath to the new file doesn't exist
	it's created
	*/
	virtual void CreateFile(const Path& path, bool recursive = false) = 0;

	//! Delete a file or directory
	/**
	If a directory is deleted each file and subfolder inside the directory is deleted.
	\param path The path of the file or directory to delete, the path must end with / to delete a directory
	*/
	virtual void DeleteFile(const Path& path) = 0;

#if 0

	//! Copies a file or directory
	/**
	A directory is copied recursive.
	\param srcPath The source path to copy from, must end with / to denote a directory
	\param dstPath The new path of the file or directory
	\param createDstPath If the subpath of the new file doesn't exist it's created
	\param replace If theres already data on the new path it's replaced by the copied data
	*/
	virtual bool CopyFile(const Path& srcPath, const Path& dstPath, bool createDstPath, bool replace) = 0;

	//! Moves a file or directory
	/**
	Moving is equal to making a copy of a file, then deleleting the original.
	This function can be used to rename a file or directory.
	A directory is moved recursive.
	\param srcPath The source path to move from, must end with / to denote a directory
	\param dstPath The new path of the file or directory
	\param createDstPath If the subpath of the new file doesn't exist it's created
	\param replace If theres already data on the new path it's replaced by the moved data
	*/
	virtual bool MoveFile(const Path& srcPath, const Path& dstPath, bool createDstPath, bool replace) = 0;
#endif

	//! Get the folder archive representing the working directory.
	virtual StrongRef<Archive> GetRootArchive() = 0;

	//! Create a archive for the path.
	/**
	If the path points to a directory, a folder archive is created.
	Otherwise the type of archive depend on the file contents. (i.e. zip, tar, etc.)
	*/
	virtual StrongRef<Archive> CreateArchive(const Path& path) = 0;

	//! Add a mount point
	/**
	\param point The path to the mount-point.
	\param archive The archive loaded into the mount-point.
	*/
	virtual void AddMountPoint(const Path& point, Archive* archive) = 0;

	//! Remove a mount-point.
	/**
	\param point The path to the mount-point.
	\param archive The archive to remove from the mount-point,
	if NULL remove the whole mount-point.
	*/
	virtual void RemoveMountPoint(const Path& point, Archive* archive = nullptr) = 0;
};

}

DECLARE_FLAG_CLASS(io::EFileMode);

}


#endif
