#include "serial/MemorySerializer.h"

namespace lux
{
namespace serial
{

StrongRef<Serializer> CreateBinaryMemorySerializer(core::RawMemory& destination, StructuralTable* table, ObjectMap* map)
{
	return LUX_NEW(MemorySerializer)(destination, table, map);
}

} // namespace serial
} // namespace lux