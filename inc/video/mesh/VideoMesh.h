#ifndef INCLUDED_LUX_MESH_H
#define INCLUDED_LUX_MESH_H
#include "math/AABBox.h"

#include "core/Referable.h"

#include "video/VertexFormat.h"
#include "video/VideoEnums.h"


namespace lux
{
namespace video
{
class Geometry;
class Material;
class Renderer;

class MeshExData : public ReferenceCounted 
{
	friend class Mesh;
private:
	StrongRef<MeshExData> m_NextData;
};

class MeshExDataBoneTable : public MeshExData
{
public:
	int maxSkinWeightPerVertex;
	core::Array<core::String> boneNames;
};

class MeshExDataVertexDuplicationIndizes : public MeshExData
{
public:
	int unduplicatedCount;
	core::Array<int> indices;
};

//! A complex mesh composed of simple Sub-Meshs
class Mesh : public core::Referable
{
	LX_REFERABLE_MEMBERS_API(Mesh, LUX_API);
private:
	LUX_API Mesh();
public:
	LUX_API Mesh(Geometry* geo, Material* mat);
	LUX_API ~Mesh();

	LUX_API const Geometry* GetGeometry() const;
	LUX_API StrongRef<Geometry> GetGeometry();
	LUX_API void SetGeometry(Geometry* geo);

	LUX_API const math::AABBoxF& GetBoundingBox() const;

	//! Recalculates the bounding box based on the geometry
	/**
	If a user-defined bounding box is set, it will be deleted
	*/
	LUX_API void RecalculateBoundingBox();

	//! Set a user-defined bounding box
	LUX_API void SetBoundingBox(const math::AABBoxF& box);

	//! Set material for complete geometry.
	/**
	Removes all other materials and indices.
	*/
	LUX_API void SetMaterial(Material* m);
	//! Set material for index.
	/**
	index must be smaller than material count.
	*/
	LUX_API void SetMaterial(int index, Material* m);
	LUX_API const Material* GetMaterial(int index) const;
	LUX_API Material* GetMaterial(int index);
	LUX_API int GetMaterialCount() const;

	//! Set material range.
	/**
	Add a new material range.
	If the material is already in the list, the existing slot is used, otherwise a new slot is created.
	All primitives in range, are removed from their current material and added to the new slot.
	*/
	LUX_API void SetMaterialRange(Material* m, int firstPrimitive, int lastPrimitive);
	//! Set material range.
	/**
	Add a new material range.
	If the material is already in the list, the existing slot is used, otherwise a new slot is created.
	All primitives in range, are removed from their current material and added to the new slot.
	*/
	LUX_API void SetMaterialRange(int materialIndex, int firstPrimitive, int lastPrimitive);
	LUX_API void GetMaterialRange(int rangeIndex, int& materialIndex, int& firstPrimitive, int& lastPrimitive);
	//! Get number of material ranges.
	LUX_API int GetRangeCount() const;

	void AddExData(MeshExData* data)
	{
		LX_CHECK_NULL_ARG(data);

		if(m_ExData)
			data->m_NextData = m_ExData->m_NextData;
		m_ExData = data;
	}
	void RemoveExData(MeshExData* data)
	{
		MeshExData* prev = nullptr;
		for(auto x = m_ExData; x; x = x->m_NextData) {
			if(x == data) {
				if(prev)
					prev->m_NextData = x->m_NextData;
				if(data == m_ExData)
					m_ExData = nullptr;
				break;
			}
			prev = x;
		}
	}
	
	template <typename T>	
	core::Optional<T*> GetExData(MeshExData* last = nullptr)
	{
		if(!last)
			last = m_ExData;
		for(auto x = last; x; x = x->m_NextData) {
			auto ptr = dynamic_cast<T*>(x);
			if(ptr != nullptr)
				return ptr;
		}
		return core::Optional<T*>();
	}
	
	template <typename T>	
	core::Optional<const T*> GetExData(MeshExData* last = nullptr) const
	{
		if(!last)
			last = m_ExData;
		for(auto x = last; x; x = x->m_NextData) {
			auto ptr = dynamic_cast<const T*>(x);
			if(ptr != nullptr)
				return ptr;
		}
		return core::Optional<const T*>();
	}
	
private:
	struct MaterialRange
	{
		int material;
		int begin;

		MaterialRange(int m, int b) :
			material(m),
			begin(b)
		{
		}
	};

private:
	math::AABBoxF m_BoundingBox;

	core::Array<StrongRef<Material>> m_Materials;
	core::Array<MaterialRange> m_Ranges;
	StrongRef<Geometry> m_Geometry;

	StrongRef<MeshExData> m_ExData;
};

} // namespace video
} // namespace lux

#endif