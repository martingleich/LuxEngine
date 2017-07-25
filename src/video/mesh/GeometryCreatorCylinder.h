#ifndef INCLUDED_GEOMETRY_CREATOR_CYLINDER_H
#define INCLUDED_GEOMETRY_CREATOR_CYLINDER_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorCylinder : public GeometryCreator
{
public:
	GeometryCreatorCylinder();
	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params);
	const String& GetName() const;
	const core::ParamPackage& GetParams() const;
	StrongRef<Geometry> CreateGeometry(
		float radius = 0.5f,
		float height = 1.0f,
		s32 sectors = 16,
		s32 planes = 2,
		s32 texX = 1,
		s32 texY = 1,
		bool inside = false);

private:
	core::ParamPackage m_Package;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_CYLINDER_H