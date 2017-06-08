#ifndef INCLUDED_FILESYSTEM_WIN32_H
#define INCLUDED_FILESYSTEM_WIN32_H
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
	StrongRef<File> OpenFile(const FileDescription& desc, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	StrongRef<File> OpenFile(const path& filename, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	StrongRef<File> OpenVirtualFile(void* memory, u32 size, const string& name, bool deleteOnDrop);
	bool ExistFile(const path& filename) const;
	bool ExistDirectory(const path& filename) const;
	path GetAbsoluteFilename(const path& filename) const;
	const path& GetWorkingDirectory() const;

	File* CreateTemporaryFile(u32 Size);
	FileDescription GetFileDescription(const path& name);

	StrongRef<INIFile> CreateINIFile(const path& filename);
	StrongRef<INIFile> CreateINIFile(File* file);

	StrongRef<File> OpenLimitedFile(File* file, u32 start, u32 Size, const string& name);

	bool CreateFile(const path& path, bool recursive = false);
	/*
	bool DeleteFile(const path& path);
	bool CopyFile(const path& srcPath, const path& dstPath, bool createDstPath, bool replace);
	bool MoveFile(const path& srcPath, const path& dstPath, bool createDstPath, bool replace);
	*/

	StrongRef<Archive> GetRootArchive();
	StrongRef<Archive> CreateArchive(const path& path);

	void AddMountPoint(const path& point, Archive* archive);
	void RemoveMountPoint(const path& point, Archive* archive = nullptr);

private:
	string GetFileOpenString(EFileMode mode) const;
	Win32Path ConvertPathToWin32WidePath(const path& p) const;
	u32 GetWin32FileAttributes(const path& p) const;

	bool CreateWin32File(Win32Path& path, bool recursive = false);
	bool CreateWin32Directory(Win32Path& path, bool recursive = false);

private:
	struct MountEntry
	{
		path mountpoint;
		StrongRef<Archive> archive;
	};

private:
	path m_WorkingDirectory;

	StrongRef<ArchiveFolderWin32> m_RootArchive;
	core::array<MountEntry> m_Mounts;
};

}
}

#endif // LUX_WINDOWS
#endif
