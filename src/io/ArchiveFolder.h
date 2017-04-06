#ifndef INCLUDED_ARCHIVEFOLDER
#define INCLUDED_ARCHIVEFOLDER
#include "io/ArchiveLoader.h"
#include "core/lxArray.h"

namespace lux
{
namespace io
{

class FileSystem;
using Win32Path = core::array<u16>;

class ArchiveFolder : public Archive
{
public:
	ArchiveFolder(io::FileSystem* fileSystem, const path& dir);
	~ArchiveFolder();
	StrongRef<File> OpenFile(const path& p, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	StrongRef<File> OpenFile(const FileDescription& file, EFileMode mode = EFileMode::Read, bool createIfNotExist = false);
	bool ExistFile(const path& p);
	StrongRef<FileEnumerator> EnumerateFiles(const path& subDir = string::EMPTY);
	EArchiveCapabilities GetCaps() const;
	bool IsValid() const;
	path GetAbsolutePath(const path& p);

private:
	void SetPath(const path& dir);
	Win32Path ConvertPathToWin32WidePath(const path& p) const;
	u32 GetWin32FileAttributes(const path& p) const;

private:
	struct SelfData;
	SelfData* self;
};

class ArchiveLoaderFolder : public ArchiveLoader
{
public:
	ArchiveLoaderFolder(FileSystem* fileSystem);
	~ArchiveLoaderFolder();

	bool CanLoadFile(const path& p)
	{
		LUX_UNUSED(p);
		return true;
	}

	bool CanLoadFile(File* f)
	{
		LUX_UNUSED(f);
		return false;
	}

	StrongRef<Archive> LoadArchive(const path& p);
	StrongRef<Archive> LoadArchive(File* f)
	{
		LUX_UNUSED(f);
		return nullptr;
	}

public:
	struct SelfData;
	SelfData* self;
};


}
}

#endif // #ifndef INCLUDED_ARCHIVEFOLDER