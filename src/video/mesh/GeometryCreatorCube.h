#ifndef INCLUDED_GEOMETRY_CREATOR_CUBE_H
#define INCLUDED_GEOMETRY_CREATOR_CUBE_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorCube : public GeometryCreator
{
public:
	GeometryCreatorCube();

	const string& GetName() const;
	const core::ParamPackage& GetParams() const;

	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params);

	//! Create a cube
	/**
	\param sizeX The x size of the cube, must be positive.
	\param sizeY The y size of the cube, must be positive.
	\param sizeZ The z size of the cube, must be positive.
	\param tesX The number of tesselations on the x axis, must be at least 2.
	\param tesY The number of tesselations on the y axis, must be at least 2.
	\param tesZ The number of tesselations on the z axis, must be at least 2.
	\param texX The number of texture repeats on the x axis.
	\param texY The number of texture repeats on the y axis.
	\param texZ The number of texture repeats on the z axis.
	\param inside If true, the normals point inside the cube, otherwise they show outside.
	\return The created plane
	*/
	StrongRef<Geometry> CreateGeometry(
		float sizeX, float sizeY, float sizeZ,
		s32 tesX, s32 tesY, s32 tesZ,
		float texX, float texY, float texZ,
		bool inside);

private:
	core::ParamPackage m_Package;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_CUBE_H