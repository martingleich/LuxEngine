#include "video/mesh/VideoMesh.h"
#include "video/Material.h"
#include "video/mesh/Geometry.h"

LX_REGISTER_REFERABLE_CLASS(lux::video::Mesh, "lux.resource.Mesh");


namespace lux
{
namespace video
{

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::RecalculateBoundingBox()
{
	if(m_Geometry)
		m_BoundingBox = m_Geometry->GetBoundingBox();
	else
		m_BoundingBox.Set(0, 0, 0);
}

void Mesh::SetMaterial(Material* m)
{
	m_Ranges.Clear();
	m_Materials.Clear();
	m_Materials.PushBack(m);
	m_Ranges.PushBack(MaterialRange(0, 0));
}

void Mesh::SetMaterial(int index, Material* m)
{
	m_Materials.At(index) = m;
}

const Material* Mesh::GetMaterial(int index) const
{
	return m_Materials.At(index);
}

Material* Mesh::GetMaterial(int index)
{
	return m_Materials.At(index);
}

int Mesh::GetMaterialCount() const
{
	return m_Materials.Size();
}

void Mesh::SetMaterialRange(Material* mat, int firstPrimitive, int lastPrimitive)
{
	int mid = 0;
	for(auto m : m_Materials) {
		if(m == mat)
			break;
		++mid;
	}

	if(mid == m_Materials.Size())
		m_Materials.PushBack(mat);

	SetMaterialRange(mid, firstPrimitive, lastPrimitive);
}

void Mesh::SetMaterialRange(int mid, int firstPrimitive, int lastPrimitive)
{
	if(firstPrimitive > lastPrimitive)
		return;

	if(m_Ranges.IsEmpty()) {
		m_Ranges.PushBack(MaterialRange(mid, firstPrimitive));
		return;
	}

	/*
	Delete every range intesection with the new one.
	And remember their materials(mid1,mid2), and the start
	of the first one(begin1) and the id where the first one was located
	insertAt.
	*/
	bool inside = false;
	int insertAt = 0;
	int begin1 = 0;
	int mid1 = 0, mid2 = 0;
	for(int i = 0; i < m_Ranges.Size(); ++i) {
		if(!inside && m_Ranges[i].begin <= firstPrimitive && (m_Ranges.Size() == i + 1 || firstPrimitive <= m_Ranges[i + 1].begin)) {
			insertAt = i;
			inside = true;
			begin1 = m_Ranges[i].begin;
			mid1 = mid2 = m_Ranges[i].material;
		}
		if(m_Ranges[i].begin <= lastPrimitive && (m_Ranges.Size() == i + 1 || m_Ranges[i + 1].begin > lastPrimitive)) {
			mid2 = m_Ranges[i].material;
			m_Ranges.EraseHoldOrder(i);
			break;
		}
		if(inside) {
			m_Ranges.EraseHoldOrder(i);
			--i;
		}
	}

	/*
	Add the ranges back.
	Add a range for the remaining piece before the new one.
	Add the new one.
	Add a range for the remaining piece after the new one.

	If a range is empty it's not added.
	If two neigbouring ranges have the same type, the second one
	isn't added.
	*/
	if(begin1 != firstPrimitive) {
		m_Ranges.Insert(MaterialRange(mid1, begin1), insertAt);
		insertAt++;
	}
	if(insertAt > 0 && m_Ranges[insertAt - 1].material == mid) {
		(void)0;
	} else {
		m_Ranges.Insert(MaterialRange(mid, firstPrimitive), insertAt);
		++insertAt;
	}

	auto geo = GetGeometry();
	int primCount = geo ? geo->GetPrimitiveCount() : 0xFFFFFFFF;
	if(lastPrimitive < primCount) {
		if(insertAt > 0 && m_Ranges[insertAt - 1].material == mid2) {
			(void)0;
		} else {
			m_Ranges.Insert(MaterialRange(mid2, lastPrimitive + 1), insertAt);
		}
	}

	// Remove unused materials
	core::Array<bool> used;
	used.Resize(m_Materials.Size(), false);
	int count = 0;
	for(int i = 0; i < m_Ranges.Size(); ++i) {
		if(used[m_Ranges[i].material] == false)
			++count;
		used[m_Ranges[i].material] = true;
	}

	if(count != m_Materials.Size()) {
		core::Array<int> replaceTable;
		replaceTable.Resize(m_Materials.Size());
		for(int i = 0; i < m_Materials.Size(); ++i) {
			if(!used[i]) {
				int idx = i;
				while(idx < used.Size() && !used[idx])
					++idx;
				if(idx < used.Size()) {
					replaceTable[i] = idx;
					m_Materials[i] = m_Materials[idx];
					used[idx] = false;
					used[i] = true;
				}
			} else {
				replaceTable[i] = i;
			}
		}

		for(auto& r : m_Ranges) {
			r.material = replaceTable[r.material];
		}
		m_Materials.Resize(count);
	}
}

void Mesh::GetMaterialRange(int rangeIndex, int& materialIndex, int& firstPrimitive, int& lastPrimitive)
{
	auto geo = GetGeometry();
	int primCount = geo ? geo->GetPrimitiveCount() : 0;
	auto& range = m_Ranges.At(rangeIndex);
	materialIndex = range.material;
	firstPrimitive = range.begin;
	lastPrimitive = rangeIndex + 1 < m_Ranges.Size() ? m_Ranges[rangeIndex + 1].begin - 1 : GetGeometry()->GetPrimitiveCount() - 1;
	lastPrimitive = math::Min<int>(lastPrimitive, primCount);
}

int Mesh::GetRangeCount() const
{
	return m_Ranges.Size();
}

core::Name Mesh::GetReferableType() const
{
	return core::ResourceType::Mesh;
}

const Geometry* Mesh::GetGeometry() const
{
	return m_Geometry;
}

StrongRef<Geometry> Mesh::GetGeometry()
{
	return m_Geometry;
}

void Mesh::SetGeometry(Geometry* geo)
{
	m_Geometry = geo;
}

const math::AABBoxF& Mesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

void Mesh::SetBoundingBox(const math::AABBoxF& box)
{
	m_BoundingBox = box;
}


} // namespace video
} // namespace lux