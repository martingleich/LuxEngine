#ifndef INCLUDED_LUX_ARCHIVEFOLDER_WIN32_H
#define INCLUDED_LUX_ARCHIVEFOLDER_WIN32_H
#include "io/ArchiveLoader.h"
#include "core/lxArray.h"

#ifdef LUX_WINDOWS
#include "platform/StrippedWindows.h"

namespace lux
{
namespace io
{

class FileSystem;
using Win32Path = core::Array<u16>;

class ArchiveFolderWin32 : public Archive
{
public:
	ArchiveFolderWin32(io::FileSystem* fileSystem, const Path& dir);
	~ArchiveFolderWin32();
	StrongRef<File> OpenFile(const Path& p, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false);
	StrongRef<File> OpenFile(const FileDescription& file, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false);
	bool ExistFile(const Path& p) const;
	core::Range<FileIterator> EnumerateFiles(const Path& subDir = core::String::EMPTY);
	EArchiveCapFlag GetCaps() const;
	Path GetAbsolutePath(const Path& p) const;
	const Path& GetPath() const;

private:
	Win32Path ConvertPathToWin32WidePath(const Path& p) const;
	DWORD GetWin32FileAttributes(const Path& p) const;

private:
	struct SelfData;
	SelfData* self;
};

class ArchiveLoaderFolderWin32 : public ArchiveLoader
{
public:
	ArchiveLoaderFolderWin32(FileSystem* fileSystem);
	~ArchiveLoaderFolderWin32();

	bool CanLoadFile(const Path& p)
	{
		LUX_UNUSED(p);
		return true;
	}

	bool CanLoadFile(File* f)
	{
		LUX_UNUSED(f);
		return false;
	}

	StrongRef<Archive> LoadArchive(const Path& p);
	StrongRef<Archive> LoadArchive(File* f)
	{
		LUX_UNUSED(f);
		return nullptr;
	}

public:
	struct SelfData;
	SelfData* self;
};

}
}

#endif // LUX_WINDOWS

#endif // #ifndef INCLUDED_LUX_ARCHIVEFOLDER_H