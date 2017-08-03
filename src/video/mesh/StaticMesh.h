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
	StaticMesh(const core::ResourceOrigin& origin);
	const Geometry* GetGeometry() const;
	StrongRef<Geometry> GetGeometry();
	void SetGeometry(Geometry* geo);
	const math::AABBoxF& GetBoundingBox() const;
	void SetBoundingBox(const math::AABBoxF& box);
	void RecalculateBoundingBox();

private:
	math::AABBoxF m_BoundingBox;
	StrongRef<Geometry> m_Geometry;
};

}    //namespace video
}    //namespace lux

#endif