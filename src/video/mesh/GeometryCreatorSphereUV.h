#ifndef INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H
#define INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorSphereUV : public GeometryCreator
{
public:
	GeometryCreatorSphereUV();

	const string& GetName() const;
	const core::ParamPackage& GetParams() const;

	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params);

	//! Create a uv sphere
	/**
	\param radius The radius of the sphere
	\param rings The number of rings on the sphere
	\param sectors The number of sectors on the sphere
	\param texX The texturerepeats on the sphere in X direction
	\param texY The texturerepeats on the sphere in Y direction
	\param inside If true, the normals point inside the cube, otherwise they show outside.
	*/
	StrongRef<Geometry> CreateGeometry(
		float radius,
		s32 rings, s32 sectors, float texX, float texY,
		bool inside);

private:
	core::ParamPackage m_Params;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H