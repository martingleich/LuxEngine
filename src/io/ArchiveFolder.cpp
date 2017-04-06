#include "ArchiveFolder.h"
#include "io/FileSystem.h"
#include "io/File.h"
#include "core/lxUnicodeConversion.h"
#include "StrippedWindows.h"

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

static bool IsFilenameValid(const wchar_t* filename)
{
	if(filename[0] == L'.') {
		if(filename[1] == L'\0')
			return false;
		if(filename[1] == L'.' && filename[2] == L'\0')
			return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

class ArchiveFolderEnumerator : public FileEnumerator
{
public:
	ArchiveFolderEnumerator(
		Archive* archive,
		const path& basePath,
		const Win32Path& win32Path);
	~ArchiveFolderEnumerator();
	bool IsValid() const;
	bool Advance();
	const FileDescription& GetCurrent() const;

private:
	HANDLE m_FindHandle;
	WIN32_FIND_DATAW m_FindData;
	bool m_IsValid;

	FileDescription m_Current;
};


ArchiveFolderEnumerator::ArchiveFolderEnumerator(
	Archive* archive,
	const path& basePath,
	const Win32Path& win32Path)
{
	Win32Path win32searchPath;
	win32searchPath.Reserve(win32Path.Size() + 2);
	win32searchPath.Push_Back(win32Path);
	win32searchPath.Push_Back(L'*');
	win32searchPath.Push_Back(0);

	m_FindHandle = FindFirstFileW((const wchar_t*)win32searchPath.Data_c(), &m_FindData);
	m_IsValid = (m_FindHandle != nullptr);

	while(m_IsValid && !IsFilenameValid(m_FindData.cFileName))
		m_IsValid = (FindNextFileW(m_FindHandle, &m_FindData) == TRUE);

	if(m_IsValid) {
		m_Current.SetIsVirtual(false);
		m_Current.SetArchive(archive);
		m_Current.SetPath(basePath);
		m_Current.SetName(core::UTF16ToString(m_FindData.cFileName));
		m_Current.SetSize(m_FindData.nFileSizeLow);
		m_Current.SetType(GetFileTypeFromAttributes(m_FindData.dwFileAttributes));
		core::DateAndTime date;
		ConvertWin32FileTimeToLuxTime(m_FindData.ftCreationTime, date);
		m_Current.SetCreationDate(date);
	}
}

ArchiveFolderEnumerator::~ArchiveFolderEnumerator()
{
	if(m_FindHandle)
		FindClose(m_FindHandle);
}

bool ArchiveFolderEnumerator::IsValid() const
{
	return m_IsValid;
}

bool ArchiveFolderEnumerator::Advance()
{
	if(!m_IsValid)
		return false;

	do {
		m_IsValid = (FindNextFileW(m_FindHandle, &m_FindData) == TRUE);
	} while(m_IsValid && !IsFilenameValid(m_FindData.cFileName));

	if(m_IsValid) {
		m_Current.SetName(core::UTF16ToString(m_FindData.cFileName));
		m_Current.SetSize(m_FindData.nFileSizeLow);
		m_Current.SetType(GetFileTypeFromAttributes(m_FindData.dwFileAttributes));

		core::DateAndTime date;
		ConvertWin32FileTimeToLuxTime(m_FindData.ftCreationTime, date);
		m_Current.SetCreationDate(date);
	}

	return m_IsValid;
}

const FileDescription& ArchiveFolderEnumerator::GetCurrent() const
{
	return m_Current;
}

///////////////////////////////////////////////////////////////////////////////

struct ArchiveLoaderFolder::SelfData
{
	WeakRef<FileSystem> fileSys;
};

ArchiveLoaderFolder::ArchiveLoaderFolder(FileSystem* fileSystem) :
	self(LUX_NEW(SelfData))
{
	self->fileSys = fileSystem;
}

ArchiveLoaderFolder::~ArchiveLoaderFolder()
{
	LUX_FREE(self);
}

StrongRef<Archive> ArchiveLoaderFolder::LoadArchive(const path& p)
{
	StrongRef<Archive> a = LUX_NEW(ArchiveFolder)(self->fileSys, p);
	if(a->IsValid())
		return a;
	else
		return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

struct ArchiveFolder::SelfData
{
	string name;
	path path;

	Win32Path win32AbsPath;

	bool isValid;

	WeakRef<io::FileSystem> fileSystem;
};

ArchiveFolder::ArchiveFolder(io::FileSystem* fileSystem, const path& dir) :
	self(LUX_NEW(SelfData))
{
	self->fileSystem = fileSystem;
	SetPath(dir);
}

ArchiveFolder::~ArchiveFolder()
{
	LUX_FREE(self);
}

StrongRef<File> ArchiveFolder::OpenFile(const path& p, EFileMode mode, bool createIfNotExist)
{
	if(self->fileSystem) {
		return self->fileSystem->OpenFile(self->path + p, mode, createIfNotExist);
	} else {
		return nullptr;
	}
}

StrongRef<File> ArchiveFolder::OpenFile(const FileDescription& file, EFileMode mode, bool createIfNotExist)
{
	if(file.GetArchive() == this) {
		return OpenFile(file.GetPath() + file.GetName(), mode, createIfNotExist);
	} else {
		return nullptr;
	}
}

bool ArchiveFolder::ExistFile(const path& p)
{
#ifdef LUX_WINDOWS
	DWORD fatt = GetWin32FileAttributes(p);
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
#error Not implemented
#endif
}

StrongRef<FileEnumerator> ArchiveFolder::EnumerateFiles(const path& subDir)
{
	path correctedSubDir = NormalizePath(subDir, true);
	path fullPath = self->path + correctedSubDir;
	Win32Path win32Path = ConvertPathToWin32WidePath(fullPath);

	return LUX_NEW(ArchiveFolderEnumerator)(this, correctedSubDir, win32Path);
}

EArchiveCapabilities ArchiveFolder::GetCaps() const
{
	return EArchiveCapabilities::Read | EArchiveCapabilities::Add | EArchiveCapabilities::Delete | EArchiveCapabilities::Change;
}

bool ArchiveFolder::IsValid() const
{
	return self->isValid;
}

path ArchiveFolder::GetAbsolutePath(const path& p)
{
	return (self->path + p);
}

void ArchiveFolder::SetPath(const path& dir)
{
	if(dir.IsEmpty() || self->fileSystem->ExistDirectory(dir)) {
		self->path = NormalizePath(dir, true);
		self->win32AbsPath = ConvertPathToWin32WidePath(self->path);
		self->isValid = true;
	} else {
		self->isValid = false;
	}
}

core::array<u16> ArchiveFolder::ConvertPathToWin32WidePath(const path& p) const
{
	core::array<u16> p2 = core::UTF8ToUTF16(p.Data());

	for(auto it = p2.First(); it != p2.End(); ++it)
		if(*it == L'/')
			*it = L'\\';

	core::array<u16> out;
	out.Reserve(4 + self->win32AbsPath.Size() + p2.Size() + 1);
	if(!self->win32AbsPath.IsEmpty())
		out.Push_Back(self->win32AbsPath.Data(), self->win32AbsPath.Size() - 1);
	out.Push_Back(p2.Data(), p2.Size());

	return out;
}

u32 ArchiveFolder::GetWin32FileAttributes(const path& p) const
{
	Win32Path win32Path = ConvertPathToWin32WidePath(p);
	return (u32)GetFileAttributesW((wchar_t*)win32Path.Data_c());
}

}
}
