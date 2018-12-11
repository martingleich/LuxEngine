#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "io/FileSystemWin32.h"

#include "math/lxMath.h" 
#include "core/Logger.h"
#include "core/lxArray.h"

#include "io/MemoryFile.h"
#include "io/StreamFile.h"
#include "io/LimitedFile.h"

#include "io/INIFile.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxSTDIO.h"

#ifdef LUX_WINDOWS
#include <direct.h>
#include "platform/StrippedWindows.h"
#include "platform/Win32Exception.h"
#endif

#include <stdlib.h>

#ifdef LUX_LINUX
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace lux
{
namespace io
{

static Path GetExecutableDirectory()
{
	// Get the path of the executable which loaded the engine.
	Win32Path path;
	DWORD ret;
	path.Resize(MAX_PATH);
	while(true) {
		DWORD nSize = (DWORD)path.Size();
		ret = GetModuleFileNameW(NULL, (wchar_t*)path.Data(), nSize);
		if(ret == nSize) {
			DWORD err = GetLastError();
			if(err == ERROR_INSUFFICIENT_BUFFER)
				path.Resize((path.Allocated() + 1) * 2);
			else
				throw core::Win32Exception(err);
		} else {
			path.Resize(ret);
			break;
		}
	}
	const u16* rawData;
	// The path may be a kernel path, so remove that.
	if(path.Size() >= 4 && path[0]=='\\' && path[1] == '\\' && path[2] == '?' && path[3] == '\\')
		rawData = path.Data()+4;
	else
		rawData = path.Data();
	return io::Path(core::UTF16ToString(rawData, -1)).GetFileDir();
}

//////////////////////////////////////////////////////////////////////////////////////

Path FileSystemWin32::ResolveMountPoints(const Path& p) const
{
	if(p.GetArchive())
		return p;

	auto view = p.AsView();
	for(auto& mount : m_Mounts) {
		auto mview = mount.mountpoint.AsView();
		if(view.StartsWith(mview))
			return Path(view.EndSubString(mview.Size()), mount.archive);
	}
	return Path(view, m_RootArchive);
}

FileSystemWin32::FileSystemWin32()
{
	m_WorkingDirectory = GetExecutableDirectory();
	m_RootArchive = LUX_NEW(ArchiveFolderWin32)(m_WorkingDirectory);
}

StrongRef<File> FileSystemWin32::OpenFile(const Path& filename, EFileModeFlag mode, bool createIfNotExist)
{
	auto resolved = ResolveMountPoints(filename);
	return resolved.GetArchive()->OpenFile(resolved, mode, createIfNotExist);
}

StrongRef<File> FileSystemWin32::OpenVirtualFile(void* memory, s64 size, const Path& name, EVirtualCreateFlag flags)
{
	LX_CHECK_NULL_ARG(memory);
	LX_CHECK_NULL_ARG(size);
	FileInfo desc(size, FileInfo::EType::VirtualFile);
	return LUX_NEW(MemoryFile)(memory, desc, name, flags);
}

StrongRef<File> FileSystemWin32::OpenVirtualFile(const void* memory, s64 size, const Path& name, EVirtualCreateFlag flags)
{
	LX_CHECK_NULL_ARG(memory);
	LX_CHECK_NULL_ARG(size);
	FileInfo desc(size, FileInfo::EType::VirtualFile);
	return LUX_NEW(MemoryFile)(memory, desc, name, CombineFlags(flags, EVirtualCreateFlag::ReadOnly));
}

Path FileSystemWin32::GetAbsoluteFilename(const Path& filename) const
{
	auto p = ResolveMountPoints(filename);
	return p.GetArchive()->GetAbsolutePath(p);
}

const Path& FileSystemWin32::GetWorkingDirectory() const
{
	return m_WorkingDirectory;
}

bool FileSystemWin32::ExistFile(const Path& filename) const
{
	auto p = ResolveMountPoints(filename);
	return p.GetArchive()->ExistFile(p);
}

bool FileSystemWin32::ExistDirectory(const Path& filename) const
{
	auto p = ResolveMountPoints(filename);
	return p.GetArchive()->ExistDirectory(p);
}

File* FileSystemWin32::CreateTemporaryFile(s64 size)
{
	void* ptr = LUX_NEW_RAW(core::SafeCast<size_t>(size));
	return OpenVirtualFile(ptr, size, io::Path::EMPTY, EVirtualCreateFlag::DeleteOnDrop);
}

FileInfo FileSystemWin32::GetFileInfo(const Path& filename)
{
	auto p = ResolveMountPoints(filename);
	return p.GetArchive()->GetFileInfo(p);
}

StrongRef<INIFile> FileSystemWin32::CreateINIFile(const Path& filename)
{
	return LUX_NEW(INIFile)(filename);
}

StrongRef<INIFile> FileSystemWin32::CreateINIFile(File* file)
{
	return LUX_NEW(INIFile)(file);
}

StrongRef<File> FileSystemWin32::OpenLimitedFile(File* file, s64 start, s64 size, const Path& name)
{
	LX_CHECK_NULL_ARG(file);

	if(start + size > file->GetSize())
		throw core::GenericRuntimeException("Limited file size is to big");

	FileInfo desc(size, FileInfo::EType::Other);
	return LUX_NEW(LimitedFile)(file, start, desc, name);
}

void FileSystemWin32::CreateFile(const Path& path, bool recursive)
{
	auto p = ResolveMountPoints(path);
	p.GetArchive()->CreateFile(p, recursive);
}

void FileSystemWin32::DeleteFile(const Path& path)
{
	auto p = ResolveMountPoints(path);
	p.GetArchive()->DeleteFile(p);
}

void FileSystemWin32::CreateDirectory(const Path& path, bool recursive)
{
	auto p = ResolveMountPoints(path);
	p.GetArchive()->CreateDirectory(p, recursive);
}

void FileSystemWin32::DeleteDirectory(const Path& path)
{
	auto p = ResolveMountPoints(path);
	p.GetArchive()->DeleteDirectory(p);
}

StrongRef<Archive> FileSystemWin32::GetRootArchive()
{
	return m_RootArchive;
}

StrongRef<Archive> FileSystemWin32::CreateArchive(const Path& path)
{
	return LUX_NEW(ArchiveFolderWin32)(GetAbsoluteFilename(path));
}

void FileSystemWin32::AddMountPoint(const Path& point, Archive* archive)
{
	if(!archive || point.IsEmpty())
		return;

	MountEntry e;
	e.mountpoint = point;
	core::String str(e.mountpoint.TakeString());
	if(!str.EndsWith("/"))
		str.Append("/");
	e.mountpoint.PutString(std::move(str));
	e.archive = archive;
	m_Mounts.PushBack(e);
}

void FileSystemWin32::RemoveMountPoint(const Path& point, Archive* archive)
{
	for(auto it = m_Mounts.Begin(); it != m_Mounts.End();) {
		if(it->mountpoint == point && (!archive || archive == it->archive))
			it = m_Mounts.Erase(it, true);
		else
			++it;
	}
}

} // namespace io
} // namespace lux

#endif // LUX_WINDOWS