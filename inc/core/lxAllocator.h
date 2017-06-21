#ifndef INCLUDED_LX_ALLOCATOR_H
#define INCLUDED_LX_ALLOCATOR_H
#include "lxMemory.h"

namespace lux
{
namespace core
{

template <typename T>
struct Allocator
{
public:
	virtual ~Allocator() {}

	T* Allocate(size_t count)
	{
		return (T*)InternalNew(count * sizeof(T));
	}

	void Deallocate(T* pointer)
	{
		InternalDelete(pointer);
	}

	void Construct(T* pointer)
	{
		new ((void*)pointer) T();
	}

	void Construct(T* pointer, const T& e)
	{
		new ((void*)pointer) T(e);
	}

	void Destruct(T* pointer)
	{
		// To suppress warning(unreferenced parameter), if T doesn't have a destructur.
		(void)pointer;

		pointer->~T();
	}

protected:
	virtual void* InternalNew(size_t count)
	{
		return ::operator new(count);
	}

	virtual void InternalDelete(void* pointer)
	{
		::operator delete(pointer);
	}
};

}
}

#endif // !INCLUDED_LX_ALLOCATOR_H