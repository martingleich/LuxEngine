#include "StaticMesh.h"
#include "video/SubMesh.h"
#include "video/Material.h"
#include "core/lxAlgorithm.h"
#include "core/ReferableRegister.h"
#include "video/VideoDriver.h"

namespace lux
{
namespace scene
{

StaticMesh::StaticMesh(video::VideoDriver* driver) :
	m_BoundingBox(math::vector3f::ZERO),
	m_Driver(driver)
{
}

void StaticMesh::Clear()
{
	m_MeshBuffers.Clear();
	m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
}

size_t StaticMesh::GetSubMeshCount() const
{
	return m_MeshBuffers.Size();
}

const video::SubMesh* StaticMesh::GetSubMesh(size_t i) const
{
	return m_MeshBuffers[i];
}

StrongRef<video::SubMesh> StaticMesh::GetSubMesh(size_t i)
{
	return m_MeshBuffers[i];
}

const math::aabbox3df& StaticMesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

void StaticMesh::SetBoundingBox(const math::aabbox3df& box)
{
	m_BoundingBox = box;
}

void StaticMesh::RecalculateBoundingBox()
{
	if(m_MeshBuffers.Size() > 0) {
		m_BoundingBox = m_MeshBuffers[0]->GetBoundingBox();
		for(size_t i = 0; i < m_MeshBuffers.Size(); ++i)
			m_BoundingBox.AddBox(m_MeshBuffers[i]->GetBoundingBox());
	} else {
		m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
	}
}

StrongRef<video::SubMesh> StaticMesh::AddSubMesh(video::SubMesh* subMesh)
{
	if(subMesh)
		m_MeshBuffers.PushBack(subMesh);

	return subMesh;
}

StrongRef<video::SubMesh> StaticMesh::AddSubMesh(
	const video::VertexFormat& vertexFormat, video::EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
	video::EIndexFormat indexType, video::EHardwareBufferMapping indexHWMapping, u32 indexCount,
	video::EPrimitiveType primitiveType)
{
	return AddSubMesh(m_Driver->CreateSubMesh(
		vertexFormat, vertexHWMapping, vertexCount,
		indexType, indexHWMapping, indexCount,
		primitiveType));
}

StrongRef<video::SubMesh> StaticMesh::AddSubMesh(const video::VertexFormat& vertexFormat,
	video::EPrimitiveType primitiveType,
	u32 primitiveCount,
	bool dynamic)
{
	return AddSubMesh(m_Driver->CreateSubMesh(vertexFormat, primitiveType, primitiveCount, dynamic));
}

void StaticMesh::RemoveSubMesh(size_t index)
{
	m_MeshBuffers.Erase(m_MeshBuffers.First() + index);
}

void StaticMesh::RemoveSubMesh(video::SubMesh* subMesh)
{
	auto it = core::LinearSearch(subMesh, m_MeshBuffers.First(), m_MeshBuffers.End());
	if(it != m_MeshBuffers.End())
		m_MeshBuffers.Erase(it);
}

video::Material* StaticMesh::GetMaterial(size_t index)
{
	return m_MeshBuffers.At(index)->GetMaterial();
}

const video::Material* StaticMesh::GetMaterial(size_t index) const
{
	return m_MeshBuffers.At(index)->GetMaterial();
}

void StaticMesh::SetMaterial(size_t index, video::Material* m)
{
	m_MeshBuffers.At(index)->SetMaterial(m);
}

video::VideoDriver* StaticMesh::GetDriver() const
{
	return m_Driver;
}

core::Name StaticMesh::GetReferableSubType() const
{
	return core::ResourceType::Mesh;
}

StrongRef<Referable> StaticMesh::Clone() const
{
	return LUX_NEW(StaticMesh)(m_Driver);
}

}
}