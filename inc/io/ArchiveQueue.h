#ifndef INCLUDED_ARCHIVE_QUEUE_H
#define INCLUDED_ARCHIVE_QUEUE_H
#include "io/Archive.h"

namespace lux
{
namespace io
{

class ArchiveQueue : public Archive
{
public:
	virtual void AddArchive(Archive* a) = 0;
	virtual void RemoveArchive(Archive* a) = 0;
	virtual u32 GetArchiveCount() const = 0;
	virtual Archive* GetArchive(u32 i) = 0;
	virtual const Archive* GetArchive(u32 i) const = 0;
};

}
}

#endif // #ifndef INCLUDED_ARCHIVE_QUEUE_H