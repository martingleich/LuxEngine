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
	StaticMesh();
	~StaticMesh()
	{
	}
	void Clear();
	u32 GetSubMeshCount() const;
	const video::SubMesh* GetSubMesh(u32 i) const;
	StrongRef<video::SubMesh> GetSubMesh(u32 i);
	const math::aabbox3df& GetBoundingBox() const;
	void SetBoundingBox(const math::aabbox3df& box);
	void RecalculateBoundingBox();
	void AddSubMesh(video::SubMesh* subMesh);
	void RemoveSubMesh(u32 index);
	void RemoveSubMesh(video::SubMesh* subMesh);
	video::Material& GetMaterial(u32 index);
	const video::Material& GetMaterial(u32 index) const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	math::aabbox3df m_BoundingBox;
	core::array<StrongRef<video::SubMesh>> m_MeshBuffers;
};


}    //namespace scene
}    //namespace lux

#endif