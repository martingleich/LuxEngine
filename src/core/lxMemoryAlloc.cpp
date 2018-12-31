#include "core/lxMemoryAlloc.h"
#include "core/Logger.h"
#include <map>

namespace lux
{
namespace core
{
static std::map<const void*, MemoryDebugInfo>* g_MemoryMap = nullptr;

void InitMemoryDebugging()
{
	if(!g_MemoryMap)
		g_MemoryMap = new std::map<const void*, MemoryDebugInfo>;
}

void ExitMemoryDebugging()
{
	delete g_MemoryMap;
}

void DebugNew(const void* ptr, const MemoryDebugInfo& info)
{
	MemoryDebugInfo i = info;
	i.ptr = ptr;
	g_MemoryMap->emplace(ptr, i);
}

void DebugFree(const void* ptr, const MemoryDebugInfo& info)
{
	if(!ptr)
		return;

	auto it = g_MemoryMap->find(ptr);

	if(it == g_MemoryMap->end()) {
		log::Error("{}:{}: ERROR: Free called on not allocated pointer. ({})", info.file, info.line, ptr);
		return;
	}

	if(info.array != it->second.array) {
		log::Error("{}:{}: ERROR: Mismatching free and allocate. ({})", info.file, info.line, ptr);
		return;
	}

	g_MemoryMap->erase(it);
}

void EnumerateDebugMemoryBlocks(void(*func)(const MemoryDebugInfo& info))
{
	if(func) {
		for(auto& x : *g_MemoryMap)
			func(x.second);
	}
}

}
}
