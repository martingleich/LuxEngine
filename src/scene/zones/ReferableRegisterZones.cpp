#include "scene/zones/ZoneBox.h"
#include "scene/zones/ZoneCylinder.h"
#include "scene/zones/ZonePoint.h"
#include "scene/zones/ZoneRing.h"
#include "scene/zones/ZoneSphere.h"

#include "core/ReferableRegister.h"

const lux::core::Name lux::scene::BoxZone::TypeName("lux.zone.Box");
const lux::core::Name lux::scene::CylinderZone::TypeName("lux.zone.Cylinder");
const lux::core::Name lux::scene::PointZone::TypeName("lux.zone.Point");
const lux::core::Name lux::scene::RingZone::TypeName("lux.zone.Ring");
const lux::core::Name lux::scene::SphereZone::TypeName("lux.zone.Sphere");

LUX_REGISTER_REFERABLE_CLASS_NAMED(box, lux::scene::BoxZone::TypeName, lux::scene::BoxZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(cylinder, lux::scene::CylinderZone::TypeName, lux::scene::CylinderZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(point, lux::scene::PointZone::TypeName, lux::scene::PointZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(ring, lux::scene::RingZone::TypeName, lux::scene::RingZone);
LUX_REGISTER_REFERABLE_CLASS_NAMED(sphere, lux::scene::SphereZone::TypeName, lux::scene::SphereZone);