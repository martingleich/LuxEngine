#ifndef INCLUDED_STATICMESH_H
#define INCLUDED_STATICMESH_H
#include "scene/mesh/Mesh.h"
#include "core/lxArray.h"

namespace lux
{
namespace scene
{

class StaticMesh : public Mesh
{
public:
	StaticMesh() :
		m_Driver(nullptr)
	{
	}

	StaticMesh(video::VideoDriver* driver);
	void Clear();
	size_t GetSubMeshCount() const;
	const video::SubMesh* GetSubMesh(size_t i) const;
	StrongRef<video::SubMesh> GetSubMesh(size_t i);
	const math::aabbox3df& GetBoundingBox() const;
	void SetBoundingBox(const math::aabbox3df& box);
	void RecalculateBoundingBox();
	StrongRef<video::SubMesh> AddSubMesh(video::SubMesh* subMesh);
	StrongRef<video::SubMesh> AddSubMesh(const video::VertexFormat& vertexFormat, video::EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		video::EIndexFormat indexType, video::EHardwareBufferMapping indexHWMapping, u32 indexCount,
		video::EPrimitiveType primitiveType);
	StrongRef<video::SubMesh> AddSubMesh(const video::VertexFormat& vertexFormat = video::VertexFormat::STANDARD,
		video::EPrimitiveType primitiveType = video::EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false);
	void RemoveSubMesh(size_t index);
	void RemoveSubMesh(video::SubMesh* subMesh);
	video::Material* GetMaterial(size_t index);
	const video::Material* GetMaterial(size_t index) const;
	void SetMaterial(size_t index, video::Material* m);
	video::VideoDriver* GetDriver() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	math::aabbox3df m_BoundingBox;
	core::array<StrongRef<video::SubMesh>> m_MeshBuffers;
	video::VideoDriver* m_Driver;
};


}    //namespace scene
}    //namespace lux

#endif