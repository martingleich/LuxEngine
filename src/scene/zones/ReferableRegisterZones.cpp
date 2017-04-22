#include "scene/zones/ZoneBox.h"
#include "scene/zones/ZoneCylinder.h"
#include "scene/zones/ZonePoint.h"
#include "scene/zones/ZoneRing.h"
#include "scene/zones/ZoneSphere.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS_NAMED(box, lux::scene::BoxZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(cylinder, lux::scene::CylinderZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(point, lux::scene::PointZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(ring, lux::scene::RingZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(sphere, lux::scene::SphereZone);