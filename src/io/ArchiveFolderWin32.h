#ifndef INCLUDED_ARCHIVEFOLDER_WIN32_H
#define INCLUDED_ARCHIVEFOLDER_WIN32_H
#include "io/ArchiveLoader.h"
#include "core/lxArray.h"

#ifdef LUX_WINDOWS
namespace lux
{
namespace io
{

class FileSystem;
using Win32Path = core::array<u16>;

class ArchiveFolderWin32 : public Archive
{
public:
	ArchiveFolderWin32(io::FileSystem* fileSystem, const path& dir);
	~ArchiveFolderWin32();
	StrongRef<File> OpenFile(const path& p, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	StrongRef<File> OpenFile(const FileDescription& file, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	bool ExistFile(const path& p);
	StrongRef<FileEnumerator> EnumerateFiles(const path& subDir = string::EMPTY);
	EArchiveCapabilities GetCaps() const;
	path GetAbsolutePath(const path& p);

private:
	void SetPath(const path& dir);
	Win32Path ConvertPathToWin32WidePath(const path& p) const;
	u32 GetWin32FileAttributes(const path& p) const;

private:
	struct SelfData;
	SelfData* self;
};

class ArchiveLoaderFolderWin32 : public ArchiveLoader
{
public:
	ArchiveLoaderFolderWin32(FileSystem* fileSystem);
	~ArchiveLoaderFolderWin32();

	bool CanLoadFile(const path& p)
	{
		LUX_UNUSED(p);
		return true;
	}

	bool CanLoadFile(File* f)
	{
		LUX_UNUSED(f);
		return false;
	}

	StrongRef<Archive> LoadArchive(const path& p);
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

#endif // #ifndef INCLUDED_ARCHIVEFOLDER_H