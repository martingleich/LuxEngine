#include "io/FileSystem.h"

#include "io/FileSystemWin32.h"

namespace lux
{
namespace io
{

static StrongRef<FileSystem> g_FileSystem;
FileSystem* FileSystem::Instance()
{
#ifdef LUX_WINDOWS
	if(!g_FileSystem)
		g_FileSystem = LUX_NEW(FileSystemWin32);
	return g_FileSystem;
#else
	throw core::ErrorException("No filesystem available");
#endif
}

void FileSystem::Destroy()
{
	g_FileSystem.Reset();
}

}
}