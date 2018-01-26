#include "core/lxArray.h"

namespace lux
{
namespace core
{

void* ArrayAlloc::ArrayAllocate(size_t bytes)
{
	return ::operator new(bytes);
}

void ArrayAlloc::ArrayFree(void* ptr)
{
	::operator delete(ptr);
}

} // namespace core
} // namespace lux