#include "StaticMesh.h"
#include "video/SubMesh.h"
#include "video/Material.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace scene
{

StaticMesh::StaticMesh() :
	m_BoundingBox(math::vector3f::ZERO)
{
}

void StaticMesh::Clear()
{
	m_MeshBuffers.Clear();
	m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
}

u32 StaticMesh::GetSubMeshCount() const
{
	return m_MeshBuffers.Size();
}

const video::SubMesh* StaticMesh::GetSubMesh(u32 i) const
{
	return m_MeshBuffers[i];
}

StrongRef<video::SubMesh> StaticMesh::GetSubMesh(u32 i)
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
		for(u32 i = 0; i < m_MeshBuffers.Size(); ++i)
			m_BoundingBox.AddBox(m_MeshBuffers[i]->GetBoundingBox());
	} else {
		m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
	}
}

void StaticMesh::AddSubMesh(video::SubMesh* subMesh)
{
	if(subMesh)
		m_MeshBuffers.Push_Back(subMesh);
}

void StaticMesh::RemoveSubMesh(u32 index)
{
	m_MeshBuffers.Erase(m_MeshBuffers.First() + index);
}

void StaticMesh::RemoveSubMesh(video::SubMesh* subMesh)
{
	auto it = core::Linear_Search(subMesh, m_MeshBuffers.First(), m_MeshBuffers.End());
	if(it != m_MeshBuffers.End())
		m_MeshBuffers.Erase(it);
}

video::Material& StaticMesh::GetMaterial(u32 index)
{
	if(index < m_MeshBuffers.Size())
		return m_MeshBuffers[index]->GetMaterial();
	else
		return video::WorkMaterial;
}

const video::Material& StaticMesh::GetMaterial(u32 index) const
{
	if(index < m_MeshBuffers.Size())
		return m_MeshBuffers[index]->GetMaterial();
	else
		return video::IdentityMaterial;
}

core::Name StaticMesh::GetReferableSubType() const
{
	return core::ResourceType::Mesh;
}

StrongRef<Referable> StaticMesh::Clone() const
{
	return LUX_NEW(StaticMesh);
}

}
}