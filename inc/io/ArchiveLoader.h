#ifndef INCLUDED_LUX_ARCHIVE_LOADER_H
#define INCLUDED_LUX_ARCHIVE_LOADER_H
#include "io/Archive.h"

namespace lux
{
namespace io
{

class ArchiveLoader : public ReferenceCounted
{
public:
	virtual bool CanLoadFile(const Path& p) = 0;
	virtual bool CanLoadFile(File* f) = 0;

	virtual StrongRef<Archive> LoadArchive(const Path& p) = 0;
	virtual StrongRef<Archive> LoadArchive(File* f) = 0;
};

}
}

#endif // #ifndef INCLUDED_LUX_ARCHIVE_LOADER_H