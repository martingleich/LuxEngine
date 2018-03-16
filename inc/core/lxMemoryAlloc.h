#ifndef INCLUDED_LX_MEMORY_ALLOC_H
#define INCLUDED_LX_MEMORY_ALLOC_H
#include "core/LuxBase.h"
#include <stdlib.h>

//#define LUX_MEMORY_ALLOC_DEBUG

namespace lux
{
namespace core
{

struct MemoryDebugInfo
{
	const void* ptr;
	const char* type;
	bool array;

	const char* file;
	size_t line;

	MemoryDebugInfo(const char* _file, size_t _line, const char* _type, bool _array) :
		ptr(nullptr),
		type(_type),
		array(_array),
		file(_file),
		line(_line)
	{
	}
};

LUX_API void InitMemoryDebugging();
LUX_API void ExitMemoryDebugging();

LUX_API void DebugNew(const void* ptr, const MemoryDebugInfo& info);
LUX_API void DebugFree(const void* ptr, const MemoryDebugInfo& info);
LUX_API void EnumerateDebugMemoryBlocks(void(*func)(const MemoryDebugInfo& info));

}
}

#ifdef LUX_MEMORY_ALLOC_DEBUG
#define LUX_NEW(type) new (lux::core::MemoryDebugInfo(__FILE__, __LINE__, #type, false)) type
#define LUX_FREE(ptr) do { const auto* memory_debug_temp_ptr = (ptr); lux::core::DebugFree(memory_debug_temp_ptr, lux::core::MemoryDebugInfo(__FILE__, __LINE__, 0, false)); delete memory_debug_temp_ptr;} while(false)

#define LUX_NEW_ARRAY(type, count) new (lux::core::MemoryDebugInfo(__FILE__, __LINE__, #type, true)) type[count]
#define LUX_FREE_ARRAY(ptr) do { const auto* memory_debug_temp_ptr = (ptr); lux::core::DebugFree(memory_debug_temp_ptr, lux::core::MemoryDebugInfo(__FILE__, __LINE__, 0, true)); delete[] memory_debug_temp_ptr;} while(false)
#else
#define LUX_NEW(type) new type
#define LUX_FREE delete
#define LUX_NEW_ARRAY(type, count) new type[count]
#define LUX_FREE_ARRAY(ptr) delete[] (ptr)
#define LUX_NEW_RAW(bytes) new u8[bytes]
#define LUX_FREE_RAW(ptr) delete[] (u8*)(ptr)
#endif

inline void* operator new(std::size_t sz, const lux::core::MemoryDebugInfo& info)
{
	void* ptr = ::operator new(sz);
	lux::core::DebugNew(ptr, info);

	return ptr;
}

inline void operator delete(void* ptr, const lux::core::MemoryDebugInfo&)
{
	// Is called when the coresponding new throws.
	::operator delete(ptr);
}

namespace lux
{
namespace core
{

template <typename T>
class UniquePtr
{
public:
	UniquePtr() :
		m_Ptr(nullptr)
	{
	}
	UniquePtr(T* ptr) :
		m_Ptr(ptr)
	{
	}
	UniquePtr(const UniquePtr&) = delete;
	UniquePtr(UniquePtr&& old)
	{
		m_Ptr = old.m_Ptr;
		old.m_Ptr = nullptr;
	}

	~UniquePtr()
	{
		LUX_FREE(m_Ptr);
	}

	UniquePtr& operator=(const UniquePtr&) = delete;
	UniquePtr& operator=(UniquePtr&& old)
	{
		this->~UniquePtr();
		m_Ptr = old.m_Ptr;
		old.m_Ptr = nullptr;
		return *this;
	}

	operator T*() const
	{
		return m_Ptr;
	}
	T* operator->() const
	{
		return m_Ptr;
	}

	T* Get() const
	{
		return m_Ptr;
	}

	T* Take()
	{
		T* out = m_Ptr;
		m_Ptr = nullptr;
		return out;
	}
private:
	T* m_Ptr;
};
}
}

#endif // #ifndef INCLUDED_LX_MEMORY_ALLOC_H
