#ifndef INCLUDED_STATICMESH_H
#define INCLUDED_STATICMESH_H
#include "video/mesh/VideoMesh.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{

class StaticMesh : public Mesh
{
public:
	StaticMesh() :
		m_Driver(nullptr)
	{
	}

	StaticMesh(VideoDriver* driver);
	void Clear();
	size_t GetSubMeshCount() const;
	const Geometry* GetGeometry(size_t i) const;
	StrongRef<Geometry> GetGeometry(size_t i);
	const math::aabbox3df& GetBoundingBox() const;
	void SetBoundingBox(const math::aabbox3df& box);
	void RecalculateBoundingBox();
	StrongRef<Geometry> AddGeometry(Geometry* subMesh);
	StrongRef<Geometry> AddGeometry(const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
		EPrimitiveType primitiveType);
	StrongRef<Geometry> AddGeometry(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false);
	void RemoveGeometry(size_t index);
	void RemoveGeometry(Geometry* subMesh);
	Material* GetMaterial(size_t index);
	const Material* GetMaterial(size_t index) const;
	void SetMaterial(size_t index, Material* m);
	VideoDriver* GetDriver() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	math::aabbox3df m_BoundingBox;
	struct Entry
	{
		StrongRef<Geometry> geo;
		StrongRef<Material> mat;

		Entry(Geometry* g) :
			geo(g),
			mat(nullptr)
		{}

		bool operator==(const Entry& other) const
		{
			return geo == other.geo;
		}
	};

	core::array<Entry> m_Data;
	VideoDriver* m_Driver;
};


}    //namespace video
}    //namespace lux

#endif