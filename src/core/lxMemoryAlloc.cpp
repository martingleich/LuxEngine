#include "core/lxMemoryAlloc.h"
#include "core/Logger.h"
#include <map>

namespace lux
{
namespace core
{
std::map<const void*, MemoryDebugInfo>& GetMap()
{
	static std::map<const void*, MemoryDebugInfo>* map = nullptr;
	if(!map)
		map = new std::map<const void*, MemoryDebugInfo>;

	return *map;
}

void DebugNew(const void* ptr, const MemoryDebugInfo& info)
{
	MemoryDebugInfo i = info;
	i.ptr = ptr;
	GetMap().emplace(ptr, i);
}

void DebugFree(const void* ptr, const MemoryDebugInfo& info)
{
	if(!ptr)
		return;

	auto it = GetMap().find(ptr);

	if(it == GetMap().end()) {
		log::Error("~s:~d: ERROR: Free called on not allocated pointer. (~p)", info.file, info.line, ptr);
		return;
	}

	if(info.array != it->second.array) {
		log::Error("~s:~d: ERROR: Mismatching free and allocate. (~p)", info.file, info.line, ptr);
		return;
	}

	GetMap().erase(it);
}

void EnumerateDebugMemoryBlocks(void(*func)(const MemoryDebugInfo& info))
{
	if(func) {
		for(auto it = GetMap().begin(); it != GetMap().end(); ++it)
			func(it->second);
	}
}

}
}
