#ifndef INCLUDED_LUX_GEOMETRY_CREATOR_H
#define INCLUDED_LUX_GEOMETRY_CREATOR_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace core
{
class PackagePuffer;
class ParamPackage;
}
namespace video
{
class Geometry;

//! Interface to create geometry on the fly.
class GeometryCreator : public ReferenceCounted
{
public:
	//! Create a new geometry
	/**
	\param params The parameter which will used to create the geometry,
		must be based on the param package of the creator,
		retrieved with \ref GeometryCreator::GetParams
	\return A newly created geometry.
	*/
	virtual StrongRef<Geometry> CreateGeometry(const core::PackagePuffer& params) = 0;

	//! Get the param packge describing the parameters of the creator.
	virtual const core::ParamPackage& GetParams() const = 0;
};

}
}

#endif // #ifndef INCLUDED_LUX_GEOMETRY_CREATOR_LIB_H