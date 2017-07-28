#include "video/mesh/StaticMesh.h"
#include "video/mesh/Geometry.h"
#include "video/Material.h"

#include "video/VideoDriver.h"

#include "core/lxAlgorithm.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_RESOURCE_CLASS("lux.resource.Mesh", lux::video::StaticMesh);

namespace lux
{
namespace video
{

void StaticMesh::Clear()
{
	m_Data.Clear();
	m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
}

size_t StaticMesh::GetSubMeshCount() const
{
	return m_Data.Size();
}

const Geometry* StaticMesh::GetGeometry(size_t i) const
{
	return m_Data.At(i).geo;
}

StrongRef<Geometry> StaticMesh::GetGeometry(size_t i)
{
	return m_Data.At(i).geo;
}

const math::AABBoxF& StaticMesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

void StaticMesh::SetBoundingBox(const math::AABBoxF& box)
{
	m_BoundingBox = box;
}

void StaticMesh::RecalculateBoundingBox()
{
	if(m_Data.Size() > 0) {
		m_BoundingBox = m_Data[0].geo->GetBoundingBox();
		for(size_t i = 0; i < m_Data.Size(); ++i)
			m_BoundingBox.AddBox(m_Data[i].geo->GetBoundingBox());
	} else {
		m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
	}
}

StrongRef<Geometry> StaticMesh::AddGeometry(Geometry* subMesh = nullptr)
{
	if(subMesh)
		m_Data.PushBack(subMesh);

	return subMesh;
}

StrongRef<Geometry> StaticMesh::AddGeometry(
	const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
	EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
	EPrimitiveType primitiveType)
{
	return AddGeometry(VideoDriver::Instance()->CreateGeometry(
		vertexFormat, vertexHWMapping, vertexCount,
		indexType, indexHWMapping, indexCount,
		primitiveType));
}

StrongRef<Geometry> StaticMesh::AddGeometry(const VertexFormat& vertexFormat,
	EPrimitiveType primitiveType,
	u32 primitiveCount,
	bool dynamic)
{
	return AddGeometry(VideoDriver::Instance()->CreateGeometry(vertexFormat, primitiveType, primitiveCount, dynamic));
}

void StaticMesh::RemoveGeometry(size_t index)
{
	m_Data.Erase(m_Data.First() + index);
}

void StaticMesh::RemoveGeometry(Geometry* subMesh)
{
	Entry e(subMesh);
	auto it = core::LinearSearch(e, m_Data);
	if(it != m_Data.End())
		m_Data.Erase(it);
}

Material* StaticMesh::GetMaterial(size_t index)
{
	return m_Data.At(index).mat;
}

const Material* StaticMesh::GetMaterial(size_t index) const
{
	return m_Data.At(index).mat;
}

void StaticMesh::SetMaterial(size_t index, Material* m)
{
	m_Data.At(index).mat = m;
}

core::Name StaticMesh::GetReferableType() const
{
	return core::ResourceType::Mesh;
}

}
}