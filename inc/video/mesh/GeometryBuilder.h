#ifndef INCLUDED_LUX_GEOMETRY_BUILDER_H
#define INCLUDED_LUX_GEOMETRY_BUILDER_H
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{
class Mesh;

class GeometryBuilder : public core::Uncopyable
{
public:
	LUX_API GeometryBuilder();
	LUX_API ~GeometryBuilder();

	//! Create a new plane mesh.
	/**
	The created plane lies in the xz plane, with the local x axis
	pointing to global x, local y points to global z.
	\param sizeX The diameter of the plane in x direction.
	\param sizeY The diameter of the plane in y direction.
	\param tesX The number of tesselation in the x direction, must be at least 2.
	\param tesY The number of tesselation in the y direction, must be at least 2.
	\param texX The number of texture repeats in the x direction.
	\param texY The number of texture repeats in the y direction.
	\param function [optional] A callback function, to specifiy
	the evaluation of points on the plane.
	\param context [optional] Value passed to the callback function.
	\return A newly created plane.
	*/
	LUX_API GeometryBuilder& CreatePlane(
		float sizeX = 1.0f, float sizeY = 1.0f,
		s32 tesX = 2, s32 tesY = 2,
		float texX = 1.0f, float texY = 1.0f,
		float(*function)(void* ctx, float x, float y) = nullptr,
		void* context = nullptr);

	//! Creates a new sphere mesh
	/**
	Creates a globe like sphere
	\param radius The radius of the sphere
	\param rings The number of rings on the sphere,
		i.e. the number of longitude pieces, must be at least 2.
	\param sector The number of sectors on the sphere,
		i.e. the number of latitude pieces, must be at least 2.
	\param texX The number of texture repeats in the x direction.
	\param texY The number of texture repeats in the y direction.
	\param inside If true, the normals point inside the cube,
		otherwise they show outside.
	\return A newly created sphere.
	*/
	LUX_API GeometryBuilder& CreateUVSphere(
		float radius = 1.0f,
		s32 rings = 16, s32 sectors = 32,
		float texX = 1.0f, float texY = 1.0f,
		bool inside = false);

	//! Create a cube
	/**
	\param sizeX The x diameter of the cube, must be positive.
	\param sizeY The y diameter of the cube, must be positive.
	\param sizeZ The z diameter of the cube, must be positive.
	\param tesX The number of tesselations on the x axis, must be at least 2.
	\param tesY The number of tesselations on the y axis, must be at least 2.
	\param tesZ The number of tesselations on the z axis, must be at least 2.
	\param texX The number of texture repeats on the x axis.
	\param texY The number of texture repeats on the y axis.
	\param texZ The number of texture repeats on the z axis.
	\param inside If true, the normals point inside the cube,
		otherwise they point outside.
	\return The created plane
	*/
	LUX_API GeometryBuilder& CreateCube(
		float sizeX = 1, float sizeY = 1, float sizeZ = 1,
		s32 tesX = 2, s32 tesY = 2, s32 tesZ = 2,
		float texX = 1, float texY = 1, float texZ = 1,
		bool inside = false);

	//! Create a arrow
	/**
	\param shaft_height The height of the arrow shaft.
	\param head_height The height of the arrow head.
	\param shaft_radius The radius of the arrow shaft.
	\param head_radius The radius of the arrow head.
	\param sectors The number of sectors.
	\return The created plane
	*/
	LUX_API GeometryBuilder& CreateArrowMesh(
		float shaft_height = 2.0f, float head_height = 2.0f,
		float shaft_radius = 0.4f, float head_radius = 1.0f,
		s32 sectors = 32);

	//! Create a cylinder
	/**
	The origin of the cylinder is the center, y axis along the height.
	\param radius The radius of the cylinder.
	\param height The height of the cylinder.
	\param sectors The number of circle divison around the cylinder
		(number of cakepieces), must be bigger than two.
	\param planes The number horizontal subdivison,
		i.e. cuts through the cylinder, default is 2,
		the upper and lower cap, must be bigger than 1.
	\param texX The number of texturerepeats in the x direction
	\param texX The number of texturerepeats in the y direction
	\param inside If true the normals point inside the torus otherwise they point outside.
	\return The created cylinder
	*/
	LUX_API GeometryBuilder& CreateCylinder(
		float radius = 0.5f, float height = 1.0f,
		s32 sectors = 16, s32 planes = 2,
		s32 texX = 1, s32 texY = 1,
		bool inside = false);

	//! Create a torus
	LUX_API GeometryBuilder& CreateTorus(
		float radiusMajor = 1.0f, float radiusMinor = 0.5f,
		s32 sectorsMajor = 48, s32 sectorsMinor = 12,
		s32 texX = 1, s32 texY = 1,
		bool inside = false);

	LUX_API void Reset();
	LUX_API StrongRef<Geometry> GetPointer();
	LUX_API StrongRef<Geometry> Finalize();

private:
	LUX_API StrongRef<Geometry> GetGeoPointer(int vcount, int icount);
private:
	StrongRef<Geometry> m_Geometry;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GEOMETRY_BUILDER_H
