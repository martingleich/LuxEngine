#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "ArchiveFolderWin32.h"
#include "io/FileSystem.h"
#include "io/File.h"
#include "core/lxUnicodeConversion.h"
#include "platform/StrippedWindows.h"

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

namespace
{
class ArchiveFolderEnumerator : public AbstractFileEnumerator
{
public:
	ArchiveFolderEnumerator(
		Archive* archive,
		const Path& basePath,
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
	const Path& basePath,
	const Win32Path& win32Path)
{
	Win32Path win32searchPath;
	win32searchPath.Reserve(win32Path.Size() + 2);
	win32searchPath.PushBack(win32Path.Data(), win32Path.Size() - 1);
	win32searchPath.PushBack(L'*');
	win32searchPath.PushBack(0);

	m_FindHandle = FindFirstFileW((LPCWSTR)win32searchPath.Data_c(), &m_FindData);
	m_IsValid = (m_FindHandle != INVALID_HANDLE_VALUE);

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
}

///////////////////////////////////////////////////////////////////////////////

struct ArchiveLoaderFolderWin32::SelfData
{
	WeakRef<FileSystem> fileSys;
};

ArchiveLoaderFolderWin32::ArchiveLoaderFolderWin32(FileSystem* fileSystem) :
	self(LUX_NEW(SelfData))
{
	self->fileSys = fileSystem;
}

ArchiveLoaderFolderWin32::~ArchiveLoaderFolderWin32()
{
	LUX_FREE(self);
}

StrongRef<Archive> ArchiveLoaderFolderWin32::LoadArchive(const Path& p)
{
	return LUX_NEW(ArchiveFolderWin32)(self->fileSys, p);
}

///////////////////////////////////////////////////////////////////////////////

struct ArchiveFolderWin32::SelfData
{
	core::String name;
	Path path;

	Win32Path win32AbsPath;

	WeakRef<io::FileSystem> fileSystem;
};

ArchiveFolderWin32::ArchiveFolderWin32(io::FileSystem* fileSystem, const Path& dir) :
	self(LUX_NEW(SelfData))
{
	self->fileSystem = fileSystem;
	if(dir.IsEmpty() || self->fileSystem->ExistDirectory(dir)) {
		self->path = NormalizePath(dir, true);
		core::String win32Path = "\\\\?\\" + self->path;
		win32Path.Replace("\\", "/");
		self->win32AbsPath = core::UTF8ToUTF16(win32Path.Data());
	} else {
		throw io::FileNotFoundException(dir.Data());
	}
}

ArchiveFolderWin32::~ArchiveFolderWin32()
{
	LUX_FREE(self);
}

StrongRef<File> ArchiveFolderWin32::OpenFile(const Path& p, EFileMode mode, bool createIfNotExist)
{
	return self->fileSystem->OpenFile(self->path + p, mode, createIfNotExist);
}

StrongRef<File> ArchiveFolderWin32::OpenFile(const FileDescription& file, EFileMode mode, bool createIfNotExist)
{
	if(file.GetArchive() != this)
		throw io::FileNotFoundException("");

	return OpenFile(file.GetPath() + file.GetName(), mode, createIfNotExist);
}

bool ArchiveFolderWin32::ExistFile(const Path& p) const
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

core::Range<FileIterator> ArchiveFolderWin32::EnumerateFiles(const Path& subDir)
{
	Path correctedSubDir = NormalizePath(subDir, true);
	Win32Path win32Path = ConvertPathToWin32WidePath(correctedSubDir);

	StrongRef<AbstractFileEnumerator> enumerator = LUX_NEW(ArchiveFolderEnumerator)(this, correctedSubDir, win32Path);
	return core::MakeRange(FileIterator(enumerator), FileIterator(enumerator, -1));
}

EArchiveCapabilities ArchiveFolderWin32::GetCaps() const
{
	return EArchiveCapabilities::Read | EArchiveCapabilities::Add | EArchiveCapabilities::Delete | EArchiveCapabilities::Change;
}

Path ArchiveFolderWin32::GetAbsolutePath(const Path& p) const
{
	return (self->path + p);
}

const Path& ArchiveFolderWin32::GetPath() const
{
	return self->path;
}

u32 ArchiveFolderWin32::GetWin32FileAttributes(const Path& p) const
{
	Win32Path win32Path = ConvertPathToWin32WidePath(p);
	return (u32)GetFileAttributesW((wchar_t*)win32Path.Data_c());
}

core::Array<u16> ArchiveFolderWin32::ConvertPathToWin32WidePath(const Path& p) const
{
	core::Array<u16> out = self->win32AbsPath;
	out.PopBack(); // Remove \0 
	core::Array<u16> p2 = core::UTF8ToUTF16(p.Data());
	out.Reserve(out.Size() + p2.Size() + 1);

	for(auto it = p2.First(); it != p2.End(); ++it) {
		if(*it == L'/')
			out.PushBack(L'\\');
		else
			out.PushBack(*it);
	}

	return out;
}

}
}

#endif // LUX_WINDOWS
