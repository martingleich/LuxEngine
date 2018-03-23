#ifndef INCLUDED_LUX_MESH_H
#define INCLUDED_LUX_MESH_H
#include "math/AABBox.h"

#include "core/Resource.h"

#include "video/VertexFormat.h"
#include "video/VideoEnums.h"


namespace lux
{
namespace video
{
class Geometry;
class Material;
class Renderer;

//! A complex mesh composed of simple Sub-Meshs
class Mesh : public core::Resource
{
public:
	LUX_API Mesh(const core::ResourceOrigin& origin);
	LUX_API virtual ~Mesh();

	//! Get the geometry of the mesh
	virtual const Geometry* GetGeometry() const = 0;

	//! Get the geometry of the mesh
	virtual StrongRef<Geometry> GetGeometry() = 0;

	virtual void SetGeometry(Geometry* geo) = 0;

	//! Returns the bounding box of the mesh
	virtual const math::AABBoxF& GetBoundingBox() const = 0;

	//! Recalculates the bounding box based on the geometry
	/**
	If a user-defined bounding box is set, it will be deleted
	*/
	LUX_API virtual void RecalculateBoundingBox();

	//! Set a user-defined bounding box
	virtual void SetBoundingBox(const math::AABBoxF& box) = 0;

	LUX_API virtual void SetMaterial(Material* m);
	LUX_API virtual void SetMaterial(int index, Material* m);
	LUX_API virtual const Material* GetMaterial(int index) const;
	LUX_API virtual Material* GetMaterial(int index);
	LUX_API virtual int GetMaterialCount() const;

	LUX_API virtual void SetMaterialRange(Material* m, int firstPrimitive, int lastPrimitive);
	LUX_API virtual void SetMaterialRange(int materialIndex, int firstPrimitive, int lastPrimitive);
	LUX_API virtual void GetMaterialRange(int rangeIndex, int& materialIndex, int& firstPrimitive, int& lastPrimitive);
	LUX_API virtual int GetRangeCount() const;

	LUX_API core::Name GetReferableType() const;

private:
	struct MaterialRange
	{
		int material;
		int begin;

		MaterialRange(int m, int b) :
			material(m),
			begin(b)
		{}
	};

	core::Array<StrongRef<Material>> m_Materials;
	core::Array<MaterialRange> m_Ranges;
};

} // namespace video
} // namespace lux

#endif