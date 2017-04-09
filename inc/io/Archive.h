#ifndef INCLUDED_ARCHIVE_H
#define INCLUDED_ARCHIVE_H
#include "core/ReferenceCounted.h"
#include "io/ioConstants.h"
#include "io/path.h"
#include "core/lxIterator.h"

namespace lux
{
namespace io
{
class File;
class Archive;
/*
class LUX_API FileEnumerator
{
public:
	FileEnumerator(Archive* a, const path& path, void* ctx);
	~FileEnumerator();
	bool Valid() const;
	void operator=(const FileEnumerator&) = delete;
	bool Advance();
	const FileDescription& operator*() const;
	const FileDescription* operator->() const;

private:
	Archive* m_Archive;
	void* m_Context;
	bool m_IsValid;
	FileDescription m_CurFile;
};
*/

class FileEnumerator : public ReferenceCounted
{
public:
	virtual ~FileEnumerator() {}

	virtual bool IsValid() const = 0;
	virtual bool Advance() = 0;
	virtual const FileDescription& GetCurrent() const = 0;
};

class Archive : public ReferenceCounted
{
public:
	virtual StrongRef<File> OpenFile(const path& p, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;
	virtual StrongRef<File> OpenFile(const FileDescription& file, EFileMode mode = EFileMode::Read, bool createIfNotExist = false) = 0;
	virtual bool ExistFile(const path& p) = 0;
	virtual StrongRef<FileEnumerator> EnumerateFiles(const path& subDir = string::EMPTY) = 0;
	virtual EArchiveCapabilities GetCaps() const = 0;
	virtual bool IsValid() const = 0;
	virtual path GetAbsolutePath(const path& p) = 0;
};

}
}
#endif // #ifndef INCLUDED_ARCHIVE_H
