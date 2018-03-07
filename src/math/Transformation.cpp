#include "math/Transformation.h"

const lux::math::Transformation lux::math::Transformation::DEFAULT = lux::math::Transformation();

lux::core::Type lux::core::Types::Transformation()
{
	static lux::core::Type type(new TypeInfoTemplate<lux::math::Transformation>("Transformation", true));
	return type;
}