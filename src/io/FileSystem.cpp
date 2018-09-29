#include "io/FileSystem.h"

#include "io/FileSystemWin32.h"

namespace lux
{
namespace io
{

static StrongRef<FileSystem> g_FileSystem;

void FileSystem::Initialize(FileSystem* fileSys)
{
	if(!fileSys) {
#ifdef LUX_WINDOWS
		fileSys = LUX_NEW(FileSystemWin32);
#else
		throw core::InvalidOperationException("No filesystem available");
#endif
	}

	g_FileSystem = fileSys;
}

FileSystem* FileSystem::Instance()
{
	return g_FileSystem;
}

void FileSystem::Destroy()
{
	g_FileSystem.Reset();
}

}
}