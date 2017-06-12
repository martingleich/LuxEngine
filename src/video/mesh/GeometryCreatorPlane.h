#ifndef INCLUDED_GEOMETRYCREATOR_PLANE_H
#define INCLUDED_GEOMETRYCREATOR_PLANE_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorPlane : public GeometryCreator
{
public:
	GeometryCreatorPlane();
	const core::ParamPackage& GetParams() const;
	const string& GetName() const;

	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params);

	//! Create a tesselated plane
	/**
	\param sizeX The x size of the plane
	\param sizeY The y size of the plane
	\param tesX The number of tesselations in x direction
	\param tesY The number of tesselations in y direction
	\param texX The number of texturerepeats in the x direction
	\param texY The number of texturerepeats in y direction
	\return The created plane
	*/
	StrongRef<Geometry> CreateGeometry(
		float sizeX, float sizeY,
		s32 tesX, s32 tesY,
		float texX, float texY,
		float(*function)(void* ctx, float x, float y), void* context);

private:
	core::ParamPackage m_Params;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRYCREATOR_PLANE_H