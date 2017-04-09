#include "FileSystemImpl.h"

#include "math/lxMath.h" 
#include "core/Logger.h"
#include "core/lxArray.h"

#include "MemoryFile.h"
#include "StreamFile.h"
#include "LimitedFile.h"

#include "ArchiveFolder.h"

#include "INIFileImpl.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxSTDIO.h"

#ifdef LUX_WINDOWS
#include <direct.h>
#include "StrippedWindows.h"
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

FileSystemImpl::FileSystemImpl()
{
	Win32Path path;
	DWORD ret;
	do {
		path.Resize(MAX_PATH);
		ret = GetModuleFileNameW(NULL, (wchar_t*)path.Data(), (DWORD)path.Allocated());
		if(ret == ERROR_INSUFFICIENT_BUFFER)
			path.Resize((path.Allocated() + 1) * 2);
		else if(ret > 0) {
			path.Resize(ret);
		}
	} while(ret == ERROR_INSUFFICIENT_BUFFER);

	io::path p = core::UTF16ToString(path.Data_c());
	if(p.StartsWith("\\\\?\\"))
		p.Remove(p.First(), 4);

	m_WorkingDirectory = io::NormalizePath(io::GetFileDir(p), true);
	m_RootArchive = LUX_NEW(ArchiveFolder)(this, m_WorkingDirectory);
}

StrongRef<File> FileSystemImpl::OpenFile(const FileDescription& desc, EFileMode mode, bool createIfNotExist)
{
	if(desc.GetArchive())
		return desc.GetArchive()->OpenFile(desc, mode, createIfNotExist);

	if(desc.GetIsVirtual()) {
		if(desc.GetName().IsEmpty() == false)
			return OpenFile(desc.GetPath() + desc.GetName(), mode, createIfNotExist);
		else
			return nullptr;
	}

	return OpenFile(desc.GetPath() + desc.GetName(), mode, createIfNotExist);
}

StrongRef<File> FileSystemImpl::OpenFile(const path& filename, EFileMode mode, bool createIfNotExist)
{
	// Scan mount list
	// Found: Open file with archive.
	// Else: Open as normal file.
	if(mode == EFileMode::Read || mode == EFileMode::ReadWrite) {
		for(auto it = m_Mounts.Last(); it != m_Mounts.Begin(); --it) {
			if(filename.StartsWith(it->mountpoint)) {
				path subPath = filename.SubString(filename.First() + it->mountpoint.Length(), filename.End());
				if(it->archive->ExistFile(subPath)) {
					return it->archive->OpenFile(subPath, mode);
				}
			}
		}
	}

	FILE* file;
	FileDescription desc;

	path absPath = GetAbsoluteFilename(filename);

	if(!ExistFile(absPath)) {
		if(!createIfNotExist) {
			return nullptr;
		} else {
			log::Info("The file \"~s\" was created.", absPath);
			file = core::FOpenUTF8(absPath.Data(), GetFileOpenString(mode).Data());
			if(!file)
				return nullptr;
			GetFileDescription(absPath, desc);
			return LUX_NEW(StreamFile)(file, desc, absPath);
		}
	}

	file = core::FOpenUTF8(absPath.Data(), GetFileOpenString(mode).Data());
	if(!file)
		return nullptr;

	u32 fileSize = 0;
	if(GetFileDescription(absPath, desc))
		fileSize = desc.GetSize();

	return LUX_NEW(StreamFile)(file, desc, absPath);
}

StrongRef<File> FileSystemImpl::OpenVirtualFile(void* memory, u32 size, const string& name, bool deleteOnDrop)
{
	if(!memory || size == 0)
		return nullptr;

	FileDescription desc(
		path::EMPTY,
		string::EMPTY,
		size,
		FileDescription::EType::Other,
		core::DateAndTime(),
		true);

	return LUX_NEW(MemoryFile)(memory, desc, name, deleteOnDrop, false);
}

path FileSystemImpl::GetAbsoluteFilename(const path& filename) const
{
	for(auto it = m_Mounts.Last(); it != m_Mounts.Begin(); --it) {
		if(filename.StartsWith(it->mountpoint)) {
			path subPath = filename.SubString(filename.First() + it->mountpoint.Length(), filename.End());
			if(it->archive->ExistFile(subPath))
				return it->archive->GetAbsolutePath(subPath);
		}
	}
	return io::MakeAbsolutePath(m_WorkingDirectory, filename);
}

const path& FileSystemImpl::GetWorkingDirectory() const
{
	return m_WorkingDirectory;
}

bool FileSystemImpl::ExistFile(const path& filename) const
{
	// Scan mount-list
	for(auto it = m_Mounts.Last(); it != m_Mounts.Begin(); --it) {
		if(filename.StartsWith(it->mountpoint)) {
			path subPath = filename.SubString(filename.First() + it->mountpoint.Length(), filename.End());
			if(it->archive->ExistFile(subPath))
				return true;
		}
	}

	// Scan filesystem
#ifdef LUX_WINDOWS
	DWORD fatt = GetWin32FileAttributes(filename);
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
#error Not implemented
#endif
}

bool FileSystemImpl::ExistDirectory(const path& filename) const
{
#ifdef LUX_WINDOWS
	DWORD fatt = GetWin32FileAttributes(filename);
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
#error Not implemented.
#endif
}

File* FileSystemImpl::CreateTemporaryFile(u32 Size)
{
	void* ptr = LUX_NEW_ARRAY(u8, Size);

	return OpenVirtualFile(ptr, Size, string::EMPTY, true);
}


bool FileSystemImpl::GetFileDescription(const path& name, FileDescription& outDesc)
{
	WIN32_FIND_DATAW FindData;
	HANDLE FindHandle = FindFirstFileW((const wchar_t*)ConvertPathToWin32WidePath(name).Data_c(), &FindData);

	if(FindHandle == INVALID_HANDLE_VALUE)
		return false;

	core::DateAndTime creatinoDate;
	bool success = ConvertWin32FileTimeToLuxTime(FindData.ftCreationTime, creatinoDate);

	outDesc.SetCreationDate(creatinoDate);
	outDesc.SetIsVirtual(false);
	outDesc.SetName(io::GetFilenameOnly(name));
	outDesc.SetPath(io::GetFileDir(name));
	outDesc.SetSize(FindData.nFileSizeLow);
	outDesc.SetType(GetFileTypeFromAttributes(FindData.dwFileAttributes));

	return success;
}

StrongRef<INIFile> FileSystemImpl::CreateINIFile(const path& filename)
{
	StrongRef<INIFileImpl> out = LUX_NEW(INIFileImpl)(this);
	if(out->Init(filename))
		return out;

	return nullptr;
}

StrongRef<INIFile> FileSystemImpl::CreateINIFile(File* file)
{
	StrongRef<INIFileImpl> out = LUX_NEW(INIFileImpl)(this);
	if(out->Init(file))
		return out;

	return nullptr;
}

StrongRef<File> FileSystemImpl::OpenLimitedFile(File* file, u32 start, u32 size, const string& name)
{
	if(!file)
		return nullptr;

	if(start + size > file->GetSize())
		return nullptr;

	FileDescription desc(
		string::EMPTY,
		string::EMPTY,
		size,
		io::FileDescription::EType::Other,
		core::DateAndTime(),
		true);

	return LUX_NEW(LimitedFile)(file, start, desc, name);
}


bool FileSystemImpl::CreateFile(const path& path, bool recursive)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(path);
	if(*win32Path.Last() == L'\\')
		return CreateWin32Directory(win32Path, recursive);
	else
		return CreateWin32File(win32Path, recursive);
}


/*
// TODO
bool FileSystemImpl::DeleteFile(const path& path)
{
	return false;
}
// TODO
bool FileSystemImpl::CopyFile(const path& srcPath, const path& dstPath, bool createDstPath, bool replace)
{
	return false;
}
// TODO
bool FileSystemImpl::MoveFile(const path& srcPath, const path& dstPath, bool createDstPath, bool replace)
{
	return false;
}
*/

StrongRef<Archive> FileSystemImpl::GetRootArchive()
{
	return m_RootArchive;
}

StrongRef<Archive> FileSystemImpl::CreateArchive(const path& path)
{
	StrongRef<Archive> a = LUX_NEW(ArchiveFolder)(this, GetAbsoluteFilename(path));
	if(a->IsValid())
		return a;
	else
		return nullptr;
}

void FileSystemImpl::AddMountPoint(const path& point, Archive* archive)
{
	if(!archive || point.IsEmpty())
		return;

	if(!archive->IsValid())
		return;

	MountEntry e;
	e.mountpoint = io::NormalizePath(point, true);
	e.archive = archive;
	m_Mounts.Push_Back(e);
}

void FileSystemImpl::RemoveMountPoint(const path& point, Archive* archive)
{
	const path normal = io::NormalizePath(point, true);
	for(auto it = m_Mounts.Last(); it != m_Mounts.Begin();) {
		if(it->mountpoint == normal && (!archive || archive == it->archive))
			it = m_Mounts.Erase(it, true) - 1;
		else
			--it;
	}
}

string FileSystemImpl::GetFileOpenString(EFileMode mode) const
{
	if(mode == EFileMode::ReadWrite)
		return "r+b";

	if(TestFlag(mode, EFileMode::Read))
		return "rb";

	if(TestFlag(mode, EFileMode::Write))
		return "wb";

	return string::EMPTY;
}

Win32Path FileSystemImpl::ConvertPathToWin32WidePath(const path& p) const
{
	core::array<u16> out;
	core::array<u16> p2 = core::UTF8ToUTF16(GetAbsoluteFilename(p).Data());
	out.Reserve(4+p2.Size()+1);

	out.Push_Back((const u16*)L"\\\\?\\", 4);
	for(auto it = p2.First(); it != p2.End(); ++it) {
		if(*it == L'/')
			out.Push_Back(L'\\');
		else
			out.Push_Back(*it);
	}

	return out;
}

u32 FileSystemImpl::GetWin32FileAttributes(const path& p) const
{
	const Win32Path& win32Path = ConvertPathToWin32WidePath(p);
	return (u32)GetFileAttributesW((const wchar_t*)win32Path.Data_c());
}

bool FileSystemImpl::CreateWin32File(Win32Path& path, bool recursive)
{
	Win32Path subPath = path;
	while(*subPath.Last() != '/')
		subPath.Pop_Back();

	DWORD attrb = GetFileAttributesW((const wchar_t*)subPath.Data_c());
	bool subPathExists = false;
	if(attrb != INVALID_FILE_ATTRIBUTES) {
		if((attrb & FILE_ATTRIBUTE_DIRECTORY) != 0)
			subPathExists = true;
	}

	if(!subPathExists && recursive)
		CreateWin32Directory(subPath, true);
	else
		return false;

	HANDLE file = CreateFileW((const wchar_t*)path.Data_c(),
		0, 0, nullptr,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if(file == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(file);

	return true;
}

bool FileSystemImpl::CreateWin32Directory(Win32Path& _path, bool recursive)
{
	wchar_t* path = (wchar_t*)_path.Data_c();
	wchar_t* path_ptr = path + _path.Size();

	core::array<wchar_t*> subDirs;
	subDirs.Push_Back(path_ptr);
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
					subDirs.Push_Back(path_ptr);
				else
					break;
			} else
				break;
		}

		if(result) {
			**subDirs.Last() = L'\\';
			subDirs.Pop_Back();
		}
	} while(!subDirs.IsEmpty() && path_ptr - path > 4 && !recursive);

	return subDirs.IsEmpty();
}

}    

}    

