#ifndef INCLUDED_ARCHIVE_LOADER_H
#define INCLUDED_ARCHIVE_LOADER_H
#include "io/Archive.h"

namespace lux
{
namespace io
{

class ArchiveLoader : public ReferenceCounted
{
public:
	virtual bool CanLoadFile(const path& p) = 0;
	virtual bool CanLoadFile(File* f) = 0;

	virtual StrongRef<Archive> LoadArchive(const path& p) = 0;
	virtual StrongRef<Archive> LoadArchive(File* f) = 0;
};

}
}

#endif // #ifndef INCLUDED_ARCHIVE_LOADER_H