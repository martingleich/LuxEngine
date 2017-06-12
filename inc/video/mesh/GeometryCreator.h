#ifndef INCLUDED_GEOMETRY_CREATOR_H
#define INCLUDED_GEOMETRY_CREATOR_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"

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
class Mesh;

//! Interface to create geometry on the fly.
class GeometryCreator : public ReferenceCounted
{
public:
	virtual ~GeometryCreator()
	{}

	//! Create a new sub mesh
	/**
	\param driver The driver used to create the submesh.
	\param params The parameter which will used to create the geometry,
		must be based on the param package of the creator,
		retrieved with \ref GeometryCreator::GetParams
	\return A newly created sub mesh.
	*/
	virtual StrongRef<video::Geometry> CreateGeometry(const core::PackagePuffer& params) = 0;

	//! Get the name of the geometry creator.
	/**
	Must be unique over all geometry creators.
	\return The name of the creator.
	*/
	virtual const string& GetName() const = 0;

	//! Get the param packge describing the parameters of the creator.
	virtual const core::ParamPackage& GetParams() const = 0;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_LIB_H