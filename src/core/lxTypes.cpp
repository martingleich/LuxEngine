#include "core/lxTypes.h"

namespace lux
{
namespace core
{
const Type Type::Unknown(new TypeInfoVirtual("unknown", 0, true));

const Type Type::Integer(new TypeInfoTemplate<int>("int", true));
const Type Type::Float(new TypeInfoTemplate<float>("float", true));
const Type Type::Boolean(new TypeInfoTemplate<bool>("bool", true));
const Type Type::U32(new TypeInfoTemplate<u32>("u32", true));

}
}