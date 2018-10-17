#ifndef INCLUDED_LUX_ARCHIVEFOLDER_WIN32_H
#define INCLUDED_LUX_ARCHIVEFOLDER_WIN32_H
#ifdef LUX_WINDOWS
#include "io/ArchiveLoader.h"
#include "core/lxArray.h"

#include "platform/StrippedWindows.h"

namespace lux
{
namespace io
{

using Win32Path = core::Array<u16>;

class ArchiveFolderWin32 : public Archive
{
public:
	ArchiveFolderWin32(const Path& dir);
	~ArchiveFolderWin32();

	StrongRef<File> OpenFile(const Path& p, EFileModeFlag mode, bool createIfNotExist) override;
	bool ExistFile(const Path& p) const override;

	bool ExistDirectory(const Path& p) const override;
	FileInfo GetFileInfo(const Path& p) const override;

	void CreateFile(const Path& path, bool recursive) override;
	void DeleteFile(const Path& path) override;
	void CreateDirectory(const Path& path, bool recursive) override;
	void DeleteDirectory(const Path& path) override;

	StrongRef<AbstractFileEnumerator> EnumerateFiles(const Path& subDir) override;
	EArchiveCapFlag GetCaps() const override;
	Path GetAbsolutePath(const Path& p) const override;
	const Path& GetPath() const override;

private:
	void CreateWin32File(Win32Path& win32Path, bool recursive);
	void CreateWin32Directory(Win32Path& win32Path, bool recursive);
private:
	Path m_Path;
	Win32Path m_Win32Path;
};

class ArchiveLoaderFolderWin32 : public ArchiveLoader
{
public:
	bool CanLoadFile(const Path&) {}
	bool CanLoadFile(File*) {} 
	StrongRef<Archive> LoadArchive(const Path& p) { return LUX_NEW(ArchiveFolderWin32)(p); }
	StrongRef<Archive> LoadArchive(File*) { return nullptr; }
};

}
}

#endif // LUX_WINDOWS

#endif // #ifndef INCLUDED_LUX_ARCHIVEFOLDER_H