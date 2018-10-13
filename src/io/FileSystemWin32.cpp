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

static bool ConvertWin32FileTimeToLuxTime(FILETIME Time, core::DateAndTime& out)
{
	SYSTEMTIME SysTime;
	BOOL Result = FileTimeToSystemTime(&Time, &SysTime);
	if(!Result)
		return false;

	out.dayOfMonth = SysTime.wDay;
	out.hours = SysTime.wHour;
	out.minutes = SysTime.wMinute;
	out.month = SysTime.wMonth;
	out.seconds = SysTime.wSecond;
	out.weekDay = (core::DateAndTime::EWeekDay)SysTime.wDayOfWeek;
	out.year = SysTime.wYear;
	out.isDayLightSaving = false;

	return true;
}

static FileDescription::EType GetFileTypeFromAttributes(DWORD attrib)
{
	if(attrib & FILE_ATTRIBUTE_DIRECTORY)
		return FileDescription::EType::Directory;
	else
		return FileDescription::EType::File;
}

//////////////////////////////////////////////////////////////////////////////////////

FileSystemWin32::FileSystemWin32()
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

	io::Path p = core::UTF16ToString(path.Data_c());
	if(p.StartsWith("\\\\?\\"))
		p.Remove(p.First(), 4);

	m_WorkingDirectory = io::NormalizePath(io::GetFileDir(p), true);
	m_RootArchive = LUX_NEW(ArchiveFolderWin32)(this, m_WorkingDirectory);
}

StrongRef<File> FileSystemWin32::OpenFile(const FileDescription& desc, EFileModeFlag mode, bool createIfNotExist)
{
	if(desc.GetArchive())
		return desc.GetArchive()->OpenFile(desc, mode, createIfNotExist);

	if(desc.GetIsVirtual()) {
		if(desc.GetName().IsEmpty() == false)
			return OpenFile(desc.GetPath() + desc.GetName(), mode, createIfNotExist);
		else
			throw io::FileNotFoundException("[Unnamed file]");
	}

	return OpenFile(desc.GetPath() + desc.GetName(), mode, createIfNotExist);
}

StrongRef<File> FileSystemWin32::OpenFile(const Path& _filename, EFileModeFlag mode, bool createIfNotExist)
{
	// Scan mount list
	// Found: Open file with archive.
	// Else: Open as normal file.
	auto filename = io::NormalizePath(_filename);
	if(mode == EFileModeFlag::Read || mode == EFileModeFlag::ReadWrite) {
		for(auto it = m_Mounts.Last(); it != m_Mounts.Begin(); --it) {
			if(filename.StartsWith(it->mountpoint)) {
				Path subPath = filename.SubString(filename.Data() + it->mountpoint.Size(), filename.End());
				if(it->archive->ExistFile(subPath))
					return it->archive->OpenFile(subPath, mode);
			}
		}
	}

	FILE* file;
	FileDescription desc;

	Path absPath = GetAbsoluteFilename(filename);

	if(!ExistFile(absPath)) {
		if(!createIfNotExist) {
			throw io::FileNotFoundException(filename.Data());
		} else {
			file = core::FOpenUTF8(absPath.Data(), GetFileOpenString(mode).Data());
			if(!file)
				throw io::FileNotFoundException(filename.Data());
			log::Info("The file \"~s\" was created.", absPath);
			try {
				desc = GetFileDescription(absPath);
				return LUX_NEW(StreamFile)(file, desc, absPath);
			} catch(...) {
				fclose(file);
				throw;
			}
		}
	}

	file = core::FOpenUTF8(absPath.Data(), GetFileOpenString(mode).Data());
	if(!file)
		throw io::FileNotFoundException(filename.Data());


	try {
		desc = GetFileDescription(absPath);
		return LUX_NEW(StreamFile)(file, desc, absPath);
	} catch(...) {
		fclose(file);
		throw;
	}
}

StrongRef<File> FileSystemWin32::OpenVirtualFile(void* memory, s64 size, const core::String& name, EVirtualCreateFlag flags)
{
	if(!memory || size == 0)
		throw io::FileNotFoundException("[Empty Memory file]");

	FileDescription desc(
		Path::EMPTY,
		core::String::EMPTY,
		size,
		FileDescription::EType::Other,
		core::DateAndTime(),
		true);

	return LUX_NEW(MemoryFile)(memory, desc, name, flags);
}

StrongRef<File> FileSystemWin32::OpenVirtualFile(const void* memory, s64 size, const core::String& name, EVirtualCreateFlag flags)
{
	// File is set to read only in last parameter
	return OpenVirtualFile(const_cast<void*>(memory), size, name, flags | EVirtualCreateFlag::ReadOnly);
}

Path FileSystemWin32::GetAbsoluteFilename(const Path& _filename) const
{
	auto filename = io::NormalizePath(_filename);
	for(auto& mount : m_Mounts) {
		if(filename.StartsWith(mount.mountpoint)) {
			Path subPath = filename.SubString(filename.Data() + mount.mountpoint.Size(), filename.End());
			if(mount.archive->ExistFile(subPath))
				return mount.archive->GetAbsolutePath(subPath);
		}
	}
	return io::MakeAbsolutePath(m_WorkingDirectory, filename);
}

const Path& FileSystemWin32::GetWorkingDirectory() const
{
	return m_WorkingDirectory;
}

bool FileSystemWin32::ExistFile(const Path& _filename) const
{
	auto filename = io::NormalizePath(_filename);
	// Scan mount-list
	for(auto& mount : m_Mounts) {
		if(filename.StartsWith(mount.mountpoint)) {
			Path subPath = filename.SubString(filename.Data() + mount.mountpoint.Size(), filename.End());
			if(mount.archive->ExistFile(subPath))
				return true;
		}
	}

	// Scan filesystem
	DWORD fatt = GetWin32FileAttributes(filename);
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool FileSystemWin32::ExistFile(const FileDescription& filename) const
{
	if(filename.GetArchive())
		return filename.GetArchive()->ExistFile(filename.GetPath() + filename.GetName());

	// Scan mount-list
	for(auto& mount : m_Mounts) {
		if(filename.GetPath().StartsWith(mount.mountpoint)) {
			Path subPath = filename.GetPath().SubString(filename.GetPath().Data() + mount.mountpoint.Size(), filename.GetPath().End());
			if(mount.archive->ExistFile(subPath + filename.GetName()))
				return true;
		}
	}

	// Scan filesystem
	DWORD fatt = GetWin32FileAttributes(filename.GetPath() + filename.GetName());
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool FileSystemWin32::ExistDirectory(const Path& filename) const
{
	DWORD fatt = GetWin32FileAttributes(filename);
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

File* FileSystemWin32::CreateTemporaryFile(s64 size)
{
	void* ptr = LUX_NEW_RAW(core::SafeCast<size_t>(size));

	return OpenVirtualFile(ptr, size, core::String::EMPTY, EVirtualCreateFlag::DeleteOnDrop);
}

FileDescription FileSystemWin32::GetFileDescription(const Path& name)
{
	WIN32_FIND_DATAW FindData;
	HANDLE FindHandle = FindFirstFileW((const wchar_t*)ConvertPathToWin32WidePath(name).Data_c(), &FindData);

	if(FindHandle == INVALID_HANDLE_VALUE)
		throw io::FileNotFoundException(name.Data());

	core::DateAndTime creatinoDate;
	ConvertWin32FileTimeToLuxTime(FindData.ftCreationTime, creatinoDate);

	FileDescription outDesc;
	outDesc.SetCreationDate(creatinoDate);
	outDesc.SetIsVirtual(false);
	outDesc.SetName(io::GetFilenameOnly(name));
	outDesc.SetPath(io::GetFileDir(name));
	outDesc.SetSize(FindData.nFileSizeLow);
	outDesc.SetType(GetFileTypeFromAttributes(FindData.dwFileAttributes));

	return outDesc;
}

StrongRef<INIFile> FileSystemWin32::CreateINIFile(const Path& filename)
{
	return LUX_NEW(INIFile)(filename);
}

StrongRef<INIFile> FileSystemWin32::CreateINIFile(File* file)
{
	return LUX_NEW(INIFile)(file);
}

StrongRef<File> FileSystemWin32::OpenLimitedFile(File* file, s64 start, s64 size, const core::String& name)
{
	if(!file)
		throw io::FileNotFoundException("[Empty file]");

	if(start + size > file->GetSize())
		throw core::GenericRuntimeException("Limited file size is to big");

	FileDescription desc(
		core::String::EMPTY,
		core::String::EMPTY,
		size,
		io::FileDescription::EType::Other,
		core::DateAndTime(),
		true);

	return LUX_NEW(LimitedFile)(file, start, desc, name);
}


void FileSystemWin32::CreateFile(const Path& path, bool recursive)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(path);
	if(win32Path.Back() == L'\\')
		CreateWin32Directory(win32Path, recursive);
	else
		CreateWin32File(win32Path, recursive);
}

void FileSystemWin32::DeleteFile(const Path& path)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(path);
	if(win32Path.Back() == L'\\') {
		throw core::GenericRuntimeException("Can't remove directory");
	} else {
		auto result = DeleteFileW((LPWSTR)win32Path.Data());
		if(result == 0)
			throw core::Win32Exception(GetLastError());
	}
}

StrongRef<Archive> FileSystemWin32::GetRootArchive()
{
	return m_RootArchive;
}

StrongRef<Archive> FileSystemWin32::CreateArchive(const Path& path)
{
	return LUX_NEW(ArchiveFolderWin32)(this, GetAbsoluteFilename(path));
}

void FileSystemWin32::AddMountPoint(const Path& point, Archive* archive)
{
	if(!archive || point.IsEmpty())
		return;

	MountEntry e;
	e.mountpoint = io::NormalizePath(point, true);
	e.archive = archive;
	m_Mounts.PushBack(e);
}

void FileSystemWin32::RemoveMountPoint(const Path& point, Archive* archive)
{
	const Path normal = io::NormalizePath(point, true);
	for(auto it = m_Mounts.Last(); it != m_Mounts.Begin();) {
		if(it->mountpoint == normal && (!archive || archive == it->archive))
			it = m_Mounts.Erase(it, true) - 1;
		else
			--it;
	}
}

core::String FileSystemWin32::GetFileOpenString(EFileModeFlag mode) const
{
	if(mode == EFileModeFlag::ReadWrite)
		return "r+b";

	if(TestFlag(mode, EFileModeFlag::Read))
		return "rb";

	if(TestFlag(mode, EFileModeFlag::Write))
		return "wb";

	return core::String::EMPTY;
}

Win32Path FileSystemWin32::ConvertPathToWin32WidePath(const Path& p) const
{
	core::Array<u16> out;
	core::Array<u16> p2 = core::UTF8ToUTF16(GetAbsoluteFilename(p).Data());
	out.Reserve(4 + p2.Size() + 1);

	out.PushBack((const u16*)L"\\\\?\\", 4);
	for(auto it = p2.First(); it != p2.End(); ++it) {
		if(*it == L'/')
			out.PushBack(L'\\');
		else
			out.PushBack(*it);
	}

	return out;
}

DWORD FileSystemWin32::GetWin32FileAttributes(const Path& p) const
{
	const Win32Path& win32Path = ConvertPathToWin32WidePath(p);
	return GetFileAttributesW((const wchar_t*)win32Path.Data_c());
}

void FileSystemWin32::CreateWin32File(Win32Path& path, bool recursive)
{
	Win32Path subPath = path;
	while(*subPath.Last() != '\\')
		subPath.PopBack();
	subPath.PushBack(0);

	DWORD attrb = GetFileAttributesW((const wchar_t*)subPath.Data_c());
	bool subPathExists = false;
	if(attrb != INVALID_FILE_ATTRIBUTES) {
		if((attrb & FILE_ATTRIBUTE_DIRECTORY) != 0)
			subPathExists = true;
	}

	if(!subPathExists) {
		if(recursive)
			CreateWin32Directory(subPath, true);
		else
			throw core::GenericRuntimeException("Path does not exists.");
	}

	HANDLE file = CreateFileW((const wchar_t*)path.Data_c(),
		0, 0, nullptr,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if(file == INVALID_HANDLE_VALUE)
		throw core::Win32Exception(GetLastError());

	CloseHandle(file);
}

void FileSystemWin32::CreateWin32Directory(Win32Path& _path, bool recursive)
{
	// Each patch starts with \\?\, meaning the first four charcters are not used.

	wchar_t* path = (wchar_t*)_path.Data_c();
	wchar_t* path_ptr = path + _path.Size();

	core::Array<wchar_t*> subDirs;
	subDirs.PushBack(path_ptr);
	do {
		**subDirs.Last() = L'\0';
		BOOL result = CreateDirectoryW(path, nullptr);
		if(!result) {
			DWORD le = GetLastError();
			if(le == ERROR_ALREADY_EXISTS) {
				result = TRUE;
			} else if(le == ERROR_PATH_NOT_FOUND) {
				--path_ptr;
				while(*path_ptr != L'\\' && path_ptr - path > 4)
					--path_ptr;

				if(path_ptr - path > 4)
					subDirs.PushBack(path_ptr);
				else
					throw core::InvalidArgumentException("Filepath isn't a valid path");
			} else {
				throw core::Win32Exception(le);
			}
		}

		if(result) {
			**subDirs.Last() = L'\\';
			subDirs.PopBack();
		}
	} while(!subDirs.IsEmpty() && path_ptr - path > 4 && !recursive);
}

} // namespace io
} // namespace lux

#endif // LUX_WINDOWS