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
	const SubMesh* GetSubMesh(size_t i) const;
	StrongRef<SubMesh> GetSubMesh(size_t i);
	const math::aabbox3df& GetBoundingBox() const;
	void SetBoundingBox(const math::aabbox3df& box);
	void RecalculateBoundingBox();
	StrongRef<SubMesh> AddSubMesh(SubMesh* subMesh);
	StrongRef<SubMesh> AddSubMesh(const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
		EPrimitiveType primitiveType);
	StrongRef<SubMesh> AddSubMesh(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false);
	void RemoveSubMesh(size_t index);
	void RemoveSubMesh(SubMesh* subMesh);
	Material* GetMaterial(size_t index);
	const Material* GetMaterial(size_t index) const;
	void SetMaterial(size_t index, Material* m);
	VideoDriver* GetDriver() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	math::aabbox3df m_BoundingBox;
	core::array<StrongRef<SubMesh>> m_MeshBuffers;
	VideoDriver* m_Driver;
};


}    //namespace video
}    //namespace lux

#endif