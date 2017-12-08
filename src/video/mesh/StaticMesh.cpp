#include "video/mesh/StaticMesh.h"
#include "video/mesh/Geometry.h"

#include "video/VideoDriver.h"

LUX_REGISTER_RESOURCE_CLASS("lux.resource.Mesh", lux::video::StaticMesh);

namespace lux
{
namespace video
{

StaticMesh::StaticMesh(const core::ResourceOrigin& origin) :
	Mesh(origin)
{
}

const Geometry* StaticMesh::GetGeometry() const
{
	return m_Geometry;
}

StrongRef<Geometry> StaticMesh::GetGeometry()
{
	return m_Geometry;
}

void StaticMesh::SetGeometry(Geometry* geo)
{
	m_Geometry = geo;
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
	if(m_Geometry)
		m_BoundingBox = m_Geometry->GetBoundingBox();
	else
		m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
}

}
}