#ifndef INCLUDED_GEOMETRY_CREATOR_LIB
#define INCLUDED_GEOMETRY_CREATOR_LIB
#include "scene/mesh/MeshCache.h"

namespace lux
{
namespace core
{
class PackagePuffer;
class ParamPackage;
}
namespace video
{
class VideoDriver;
class SubMesh;
}
namespace scene
{
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
	virtual StrongRef<video::SubMesh> CreateSubMesh(video::VideoDriver* driver, const core::PackagePuffer& params) = 0;

	//! Get the name of the geometry creator.
	/**
	Must be unique over all geometry creators.
	\return The name of the creator.
	*/
	virtual const string& GetName() const = 0;

	//! Get the param packge describing the parameters of the creator.
	virtual const core::ParamPackage& GetParams() const = 0;
};

//! Interface to ease use of GeometryCreators.
class GeometryCreatorLib : public ReferenceCounted
{
public:
	virtual ~GeometryCreatorLib() {}

	//! Retrieve a geometry creator based on it's name.
	virtual StrongRef<GeometryCreator> GetByName(const string& name) const = 0;

	//! Add a new geometry creator to the library.
	/**
	The library takes ownege of the creator.
	\param creator The creator to add.
	*/
	virtual void AddCreator(GeometryCreator* creator) = 0;

	//! Removes a creator from the library.
	virtual void RemoveCreator(GeometryCreator* creator) = 0;

	//! Returns the total number of available creators.
	virtual size_t GetCreatorCount() const = 0;

	//! Returns a creator by it's index.
	virtual StrongRef<GeometryCreator> GetById(size_t id) const = 0;

	//! Creates a new param package puffer for a given creator.
	/**
	\param name The name of the geometry creator.
	\return A param package puffer matching the geometry creator
	*/
	virtual core::PackagePuffer GetCreatorParams(const string& name) = 0;

	//! Create a new mesh
	/**
	The created mesh is added anonymous to the mesh cache.
	\param name The name of the geometry creator.
	\param params The parameter for the geometry creator, retrieved by \ref GetCreatorParams
	\return A newly created mesh
	*/
	virtual StrongRef<scene::Mesh> CreateMesh(const string& name, const core::PackagePuffer& params) = 0;

	//! Create a new sub mesh.
	/**
	\param name The name of the geometry creator.
	\param params The parameter for the geometry creator, retrieved by \ref GetCreatorParams
	\return A newly created sub mesh.
	*/
	virtual StrongRef<video::SubMesh> CreateSubMesh(const string& name, const core::PackagePuffer& params) = 0;

	//! Create a new plane mesh.
	/**
	The created plane lies in the xz plane, with the local x axis pointing to global x, local y points to global z.
	\param sizeX The diameter of the plane in x direction.
	\param sizeY The diameter of the plane in y direction.
	\param tesX The number of tesselation in the x direction, must be at least 2.
	\param tesY The number of tesselation in the y direction, must be at least 2.
	\param texX The number of texture repeats in the x direction.
	\param texY The number of texture repeats in the y direction.
	\param function [optional] A callback function, to specifiy the evaluation of points on the plane.
	\param context [optional] Value passed to the callback function.
	\return A newly created plane.
	*/
	virtual StrongRef<scene::Mesh> CreatePlaneMesh(float sizeX=1.0f, float sizeY=1.0f, s32 tesX=1, s32 tesY=1, float texX=1.0f, float texY=1.0f, float(*function)(void* ctx, float x, float y) = nullptr, void* context = nullptr) = 0;

	//! Creates a new sphere mesh
	/**
	Creates a globe like sphere
	\param radius The radius of the sphere
	\param rings The number of rings on the sphere, i.e. the number of longitude pieces, must be at least 2.
	\param segements The number of segemnts on the sphere, i.e. the number of latitude pieces, must be at least 2.
	\param texX The number of texture repeats in the x direction.
	\param texY The number of texture repeats in the y direction.
	\param inside If true, the normals point inside the cube, otherwise they show outside.
	\return A newly created sphere.
	*/
	virtual StrongRef<scene::Mesh> CreateSphereMesh(float radius=1.0f, s32 rings=16, s32 segments=32, float texX=1.0f, float texY=1.0f, bool inside=false) = 0;

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
	virtual StrongRef<scene::Mesh> CreateCubeMesh( float sizeX=1.0f, float sizeY=1.0f, float sizeZ=1.0f, s32 tesX=2, s32 tesY=2, s32 tesZ=2, float texX=1.0f, float texY=1.0f, float texZ=1.0f, bool inside=false) = 0;

	//! Create a arrow
	/**
	\param shaft_height The height of the arrow shaft.
	\param head_height The height of the arrow head.
	\param shaft_radius The radius of the arrow shaft.
	\param head_radius The radius of the arrow head.
	\param sectors The number of sectors.
	\return The created plane
	*/
	virtual StrongRef<scene::Mesh> CreateArrowMesh(float shaft_height=2.0f, float head_height=2.0f, float shaft_radius=0.4f, float head_radius=1.0f, s32 sectors=32) = 0;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_LIB