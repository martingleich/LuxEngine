#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "io/ArchiveFolderWin32.h"
#include "io/FileSystem.h"
#include "io/StreamFile.h"
#include "core/lxUnicodeConversion.h"
#include "platform/WindowsUtils.h"

namespace lux
{
namespace io
{

///////////////////////////////////////////////////////////////////////////////

namespace
{
core::Array<u16> ConvertPathToWin32WidePath(const Path& p)
{
	core::Array<u16> out;
	for(auto c : p.AsView().CodePoints()) {
		u16 buffer[2];
		int bytes = core::CodePointToUTF16(c, buffer);
		if(buffer[0] == '/')
			buffer[0] = '\\';
		out.PushBack(buffer[0]);
		if(bytes == 4)
			out.PushBack(buffer[1]);
	}
	out.PushBack(0);

	return out;
}

class ArchiveFolderEnumerator : public AbstractFileEnumerator
{
public:
	ArchiveFolderEnumerator(const Path& basePath);

	bool Advance() override;
	const FileInfo& GetInfo() const override { return m_CurInfo; }
	const Path& GetBasePath() const override { return m_CurBasePath; }
	const core::String& GetName() const override { return m_CurName; }
	const Path& GetFullPath() const override
	{
		m_CurFullPath = Path(core::String(m_CurBasePath.AsView()).Append(m_CurName), m_CurBasePath.GetArchive());
		return m_CurFullPath;
	}

	static bool IsRealFile(const wchar_t* filename)
	{
		if(filename[0] == L'.') {
			if(filename[1] == L'\0')
				return false;
			if(filename[1] == L'.' && filename[2] == L'\0')
				return false;
		}

		return true;
	}

private:
	static void WrapperCloseHandle(HANDLE h) { FindClose(h); }
	HandleWrapper<WrapperCloseHandle> m_FindHandle;
	WIN32_FIND_DATAW m_FindData;
	FileInfo m_CurInfo;
	Path m_CurBasePath;
	mutable Path m_CurFullPath;
	core::String m_CurName;
	bool m_IsValid;
};

ArchiveFolderEnumerator::ArchiveFolderEnumerator(const Path& basePath)
{
	m_CurBasePath = basePath;
	m_IsValid = true;
}

bool ArchiveFolderEnumerator::Advance()
{
	if(!m_IsValid)
		return false;

	if(m_FindHandle == INVALID_HANDLE_VALUE) {
		Win32Path win32searchPath = ConvertPathToWin32WidePath(m_CurBasePath);
		win32searchPath.PopBack();
		win32searchPath.PushBack('\\');
		win32searchPath.PushBack('*');
		win32searchPath.PushBack(0);

		m_FindHandle = FindFirstFileW((LPCWSTR)win32searchPath.Data_c(), &m_FindData);
		m_IsValid = m_FindHandle == INVALID_HANDLE_VALUE;
		if(!m_IsValid)
			return false;
	}

	// Search until a valid filename is found.
	while(m_IsValid && !IsRealFile(m_FindData.cFileName))
		m_IsValid = (FindNextFileW(m_FindHandle, &m_FindData) == TRUE);

	if(m_IsValid) {
		s64 size = (s64)m_FindData.nFileSizeHigh << 32 | m_FindData.nFileSizeLow;
		auto type = FileInfo::EType::File;
		if((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			type = FileInfo::EType::Directory;
		m_CurInfo = FileInfo(size, type);
		m_CurName = core::UTF16ToString(m_FindData.cFileName, -1);
	}

	return m_IsValid;
}
}

///////////////////////////////////////////////////////////////////////////////

ArchiveFolderWin32::ArchiveFolderWin32(const Path& path)
{
	m_Path = path;
	m_Win32Path = ConvertPathToWin32WidePath(path);
}

ArchiveFolderWin32::~ArchiveFolderWin32()
{
}

StrongRef<File> ArchiveFolderWin32::OpenFile(const Path& path, EFileModeFlag mode, bool createIfNotExist)
{
	auto absDir = path.GetResolved(m_Path);
	Win32Path winPath = ConvertPathToWin32WidePath(absDir);

	DWORD access = 0;
	if(TestFlag(mode, EFileModeFlag::Read))
		access |= GENERIC_READ;
	if(TestFlag(mode, EFileModeFlag::Write))
		access |= GENERIC_WRITE;

	DWORD create = 0;
	if(createIfNotExist)
		create = OPEN_ALWAYS;
	else
		create = OPEN_EXISTING;

	Win32FileHandle file = CreateFileW((LPCWSTR)winPath.Data(), access, FILE_SHARE_READ, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);
	// TODO: Better error reporting.
	if(file == INVALID_HANDLE_VALUE)
		throw io::FileNotFoundException(absDir);

	LARGE_INTEGER size;
	if(!GetFileSizeEx(file, &size))
		size.QuadPart = 0;
	FileInfo info((s64)size.QuadPart, FileInfo::EType::File);

	return LUX_NEW(StreamFileWin32)(std::move(file), info, absDir);
}

bool ArchiveFolderWin32::ExistFile(const Path& p) const
{
	auto winPath = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	DWORD fatt = GetFileAttributesW((wchar_t*)winPath.Data_c());
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool ArchiveFolderWin32::ExistDirectory(const Path& p) const
{
	auto winPath = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	DWORD fatt = GetFileAttributesW((wchar_t*)winPath.Data_c());
	if(fatt == INVALID_FILE_ATTRIBUTES)
		return false;
	else
		return (fatt & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

FileInfo ArchiveFolderWin32::GetFileInfo(const Path& p) const
{
	auto winPath = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	DWORD fatt = GetFileAttributesW((wchar_t*)winPath.Data_c());
	if(fatt == INVALID_FILE_ATTRIBUTES) {
		return FileInfo();
	} else if((fatt & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		return FileInfo(0, FileInfo::EType::Directory);
	} else {
		Win32FileHandle file = CreateFileW((LPCWSTR)winPath.Data(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(file == INVALID_HANDLE_VALUE)
			return FileInfo();
		LARGE_INTEGER size;
		if(!GetFileSizeEx(file, &size))
			size.QuadPart = 0;
		return FileInfo((s64)size.QuadPart, FileInfo::EType::File);
	}
}

void ArchiveFolderWin32::CreateFile(const Path& p, bool recursive)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	CreateWin32File(win32Path, recursive);
}

void ArchiveFolderWin32::DeleteFile(const Path& p)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	auto result = DeleteFileW((LPWSTR)win32Path.Data());
	if(result == 0)
		throw core::Win32Exception(GetLastError());
}

void ArchiveFolderWin32::CreateDirectory(const Path& p, bool recursive)
{
	Win32Path win32Path = ConvertPathToWin32WidePath(p.GetResolved(m_Path));
	CreateWin32Directory(win32Path, recursive);
}

void ArchiveFolderWin32::DeleteDirectory(const Path& path)
{
	LUX_UNUSED(path);
	throw core::NotImplementedException("DeleteDirectory");
}

StrongRef<AbstractFileEnumerator> ArchiveFolderWin32::EnumerateFiles(const Path& subDir)
{
	Path absPath = subDir.GetResolved(m_Path);
	return LUX_NEW(ArchiveFolderEnumerator)(absPath);
}

EArchiveCapFlag ArchiveFolderWin32::GetCaps() const
{
	return CombineFlags(EArchiveCapFlag::Read, EArchiveCapFlag::Add, EArchiveCapFlag::Delete, EArchiveCapFlag::Change);
}

Path ArchiveFolderWin32::GetAbsolutePath(const Path& p) const
{
	return p.GetResolved(m_Path);
}

const Path& ArchiveFolderWin32::GetPath() const
{
	return m_Path;
}

void ArchiveFolderWin32::CreateWin32File(Win32Path& win32Path, bool recursive)
{
	Win32Path subPath = win32Path;
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

	Win32FileHandle handle = CreateFileW((const wchar_t*)win32Path.Data_c(),
		0, 0, nullptr,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if(handle == INVALID_HANDLE_VALUE)
		throw core::Win32Exception(GetLastError());
}

void ArchiveFolderWin32::CreateWin32Directory(Win32Path& win32Path, bool recursive)
{
	// Each patch starts with \\?\, meaning the first four charcters are not used.

	wchar_t* path = (wchar_t*)win32Path.Data_c();
	wchar_t* path_ptr = path + win32Path.Size();

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

void ArchiveFolderWin32::ReleaseHandles()
{
}

}
}

#endif // LUX_WINDOWS
