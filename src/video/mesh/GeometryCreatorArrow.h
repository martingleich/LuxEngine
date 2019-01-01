#ifndef INCLUDED_LUX_GEOMETRY_CREATOR_ARROW_H
#define INCLUDED_LUX_GEOMETRY_CREATOR_ARROW_H
#include "video/mesh/GeometryCreator.h"
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class GeometryCreatorArrow : public GeometryCreator
{
public:
	GeometryCreatorArrow();
	const core::ParamPackage& GetParams() const override;
	StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params) override;

	//! Create a arrow
	/**
	\param driver The video driver used to create resources.
	\param shaft_height The height of the arrow shaft.
	\param head_height The height of the arrow head.
	\param shaft_radius The radius of the arrow shaft.
	\param head_radius The radius of the arrow head.
	\param sectors The number of sectors.
	\return The created plane
	*/
	StrongRef<Geometry> CreateGeometry(
		float shaft_height, float head_height,
		float shaft_radius, float head_radius,
		s32 sectors);

private:
	core::ParamPackage m_Package;
};

}
}

#endif