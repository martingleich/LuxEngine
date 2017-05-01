#include "StaticMesh.h"
#include "video/SubMesh.h"
#include "video/Material.h"
#include "core/lxAlgorithm.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::StaticMesh)

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

void StaticMesh::AddSubMesh(video::SubMesh* subMesh)
{
	if(subMesh)
		m_MeshBuffers.PushBack(subMesh);
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

video::Material& StaticMesh::GetMaterial(size_t index)
{
	if(index < m_MeshBuffers.Size())
		return m_MeshBuffers[index]->GetMaterial();
	else
		return video::WorkMaterial;
}

const video::Material& StaticMesh::GetMaterial(size_t index) const
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