#include "core/lxArray.h"

namespace lux
{
namespace core
{

void* ArrayRawData::ArrayAllocate(size_t bytes)
{
	return ::operator new(bytes);
}

void ArrayRawData::ArrayFree(void* ptr)
{
	::operator delete(ptr);
}

} // namespace core
} // namespace lux