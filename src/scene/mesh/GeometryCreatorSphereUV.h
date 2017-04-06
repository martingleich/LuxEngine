#ifndef INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H
#define INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H
#include "scene/mesh/GeometryCreatorLib.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace scene
{

class GeometryCreatorSphereUV : public GeometryCreator
{
public:
	GeometryCreatorSphereUV();

	const string& GetName() const;
	const core::ParamPackage& GetParams() const;

	StrongRef<video::SubMesh> CreateSubMesh(video::VideoDriver* driver, const core::PackagePuffer& params);

	//! Create a uv sphere
	/**
	\param cache The mesh cache of the engine
	\param radius The radius of the sphere
	\param rings The number of rings on the sphere
	\param segments The number of segments on the sphere
	\param texX The texturerepeats on the sphere in X direction
	\param texY The texturerepeats on the sphere in Y direction
	\param inside If true, the normals point inside the cube, otherwise they show outside.
	*/
	StrongRef<video::SubMesh> CreateSubMesh(video::VideoDriver* driver,
		float radius,
		s32 rings, s32 segments, float texX, float texY,
		bool inside);

private:
	core::ParamPackage m_Params;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRYCREATOR_SPHERE_UV_H