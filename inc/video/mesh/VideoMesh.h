#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H
#include "math/AABBox.h"

#include "resources/Resource.h"

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
	LUX_API virtual void SetMaterial(size_t index, Material* m);
	LUX_API virtual const Material* GetMaterial(size_t index) const;
	LUX_API virtual Material* GetMaterial(size_t index);
	LUX_API virtual size_t GetMaterialCount() const;

	LUX_API virtual void SetMaterialRange(Material* m, size_t firstPrimitive, size_t lastPrimitive);
	LUX_API virtual void SetMaterialRange(size_t materialIndex, size_t firstPrimitive, size_t lastPrimitive);
	LUX_API virtual void GetMaterialRange(size_t rangeIndex, size_t& materialIndex, size_t& firstPrimitive, size_t& lastPrimitive);
	LUX_API virtual size_t GetRangeCount() const;

	LUX_API core::Name GetReferableType() const;

private:
	struct MaterialRange
	{
		size_t material;
		size_t begin;

		MaterialRange(size_t m, size_t b) :
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