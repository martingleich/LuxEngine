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
	StrongRef<File> OpenFile(const Path& filename, EFileModeFlag mode = EFileModeFlag::Read, bool createIfNotExist = false);
	StrongRef<File> OpenVirtualFile(void* memory, s64 size, const Path& name, EVirtualCreateFlag flag);
	StrongRef<File> OpenVirtualFile(const void* memory, s64 size, const Path& name, EVirtualCreateFlag flag);
	bool ExistFile(const Path& filename) const;
	bool ExistDirectory(const Path& filename) const;
	Path GetAbsoluteFilename(const Path& filename) const;
	FileInfo GetFileInfo(const Path& name);

	const Path& GetWorkingDirectory() const;

	File* CreateTemporaryFile(s64 Size);

	StrongRef<INIFile> CreateINIFile(const Path& filename);
	StrongRef<INIFile> CreateINIFile(File* file);

	StrongRef<File> OpenLimitedFile(File* file, s64 start, s64 size, const Path& name);

	void CreateFile(const Path& path, bool recursive);
	void DeleteFile(const Path& path);
	void CreateDirectory(const Path& path, bool recursive);
	void DeleteDirectory(const Path& path);

	StrongRef<Archive> GetRootArchive();
	StrongRef<Archive> CreateArchive(const Path& path);

	void AddMountPoint(const Path& point, Archive* archive);
	void RemoveMountPoint(const Path& point, Archive* archive = nullptr);

private:
	Path ResolveMountPoints(const Path& p) const;

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
