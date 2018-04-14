#ifndef INCLUDED_LUX_FILESYSTEM_WIN32_H
#define INCLUDED_LUX_FILESYSTEM_WIN32_H
#ifdef LUX_WINDOWS
#include "io/FileSystem.h"
#include "core/lxArray.h"

#include "io/ArchiveFolderWin32.h"

namespace lux
{
namespace io
{
class FileSystemWin32 : public FileSystem
{
public:
	FileSystemWin32();
	StrongRef<File> OpenFile(const FileDescription& desc, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false);
	StrongRef<File> OpenFile(const Path& filename, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false);
	StrongRef<File> OpenVirtualFile(void* memory, s64 size, const core::String& name, EVirtualCreateFlag flag);
	StrongRef<File> OpenVirtualFile(const void* memory, s64 size, const core::String& name, EVirtualCreateFlag flag);
	bool ExistFile(const Path& filename) const;
	bool ExistFile(const FileDescription& filename) const;
	bool ExistDirectory(const Path& filename) const;
	Path GetAbsoluteFilename(const Path& filename) const;
	const Path& GetWorkingDirectory() const;

	File* CreateTemporaryFile(s64 Size);
	FileDescription GetFileDescription(const Path& name);

	StrongRef<INIFile> CreateINIFile(const Path& filename);
	StrongRef<INIFile> CreateINIFile(File* file);

	StrongRef<File> OpenLimitedFile(File* file, s64 start, s64 size, const core::String& name);

	void CreateFile(const Path& path, bool recursive = false);
	void DeleteFile(const Path& path);
	/*
	void CopyFile(const Path& srcPath, const Path& dstPath, bool createDstPath=true, bool replace=true);
	void MoveFile(const Path& srcPath, const Path& dstPath, bool createDstPath=true, bool replace=true);
	*/

	StrongRef<Archive> GetRootArchive();
	StrongRef<Archive> CreateArchive(const Path& path);

	void AddMountPoint(const Path& point, Archive* archive);
	void RemoveMountPoint(const Path& point, Archive* archive = nullptr);

private:
	core::String GetFileOpenString(EFileModeFlag mode) const;
	Win32Path ConvertPathToWin32WidePath(const Path& p) const;
	DWORD GetWin32FileAttributes(const Path& p) const;

	void CreateWin32File(Win32Path& path, bool recursive = false);
	void CreateWin32Directory(Win32Path& path, bool recursive = false);

private:
	struct MountEntry
	{
		Path mountpoint;
		StrongRef<Archive> archive;
	};

private:
	Path m_WorkingDirectory;

	StrongRef<ArchiveFolderWin32> m_RootArchive;
	core::Array<MountEntry> m_Mounts;
};

}
}

#endif // LUX_WINDOWS
#endif
