#ifndef INCLUDED_GEOMETRY_CREATOR_TORUS_H
#define INCLUDED_GEOMETRY_CREATOR_TORUS_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorTorus : public GeometryCreator
{
public:
	GeometryCreatorTorus();
	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params);
	const string& GetName() const;
	const core::ParamPackage& GetParams() const;
	StrongRef<Geometry> CreateGeometry(
		float radiusMajor = 1.0f,
		float radiusMinor = 0.5f,
		s32 sectorsMajor = 48,
		s32 sectorsMinor = 12,
		s32 texX = 1,
		s32 texY = 1,
		bool inside = false);

private:
	core::ParamPackage m_Package;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_TORUS_H