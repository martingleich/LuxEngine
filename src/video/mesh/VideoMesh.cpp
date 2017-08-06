#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

Mesh::Mesh(const core::ResourceOrigin& origin) :
	Resource(origin)
{
}

Mesh::~Mesh()
{
}

void Mesh::RecalculateBoundingBox()
{
	SetBoundingBox(GetGeometry()->GetBoundingBox());
}

void Mesh::SetMaterial(Material* m)
{
	m_Ranges.Clear();
	m_Materials.Clear();
	m_Materials.PushBack(m);
	m_Ranges.PushBack(MaterialRange(0, 0));
}

void Mesh::SetMaterial(size_t index, Material* m)
{
	m_Materials.At(index) = m;
}

const Material* Mesh::GetMaterial(size_t index) const
{
	return m_Materials.At(index);
}

Material* Mesh::GetMaterial(size_t index)
{
	return m_Materials.At(index);
}

size_t Mesh::GetMaterialCount() const
{
	return m_Materials.Size();
}

void Mesh::SetMaterialRange(Material* mat, size_t firstPrimitive, size_t lastPrimitive)
{
	u32 mid = 0;
	for(auto m : m_Materials) {
		if(m == mat)
			break;
		++mid;
	}

	if(mid == m_Materials.Size())
		m_Materials.PushBack(mat);

	SetMaterialRange(mid, firstPrimitive, lastPrimitive);
}

void Mesh::SetMaterialRange(size_t mid, size_t firstPrimitive, size_t lastPrimitive)
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
	size_t insertAt = 0;
	size_t begin1 = 0;
	u32 mid1 = 0, mid2 = 0;
	for(size_t i = 0; i < m_Ranges.Size(); ++i) {
		if(!inside && m_Ranges[i].begin <= firstPrimitive && (m_Ranges.Size() == i + 1 || firstPrimitive <= m_Ranges[i+1].begin)) {
			insertAt = i;
			inside = true;
			begin1 = m_Ranges[i].begin;
			mid1 = mid2 = m_Ranges[i].material;
		}
		if(m_Ranges[i].begin <= lastPrimitive && (m_Ranges.Size() == i + 1 || m_Ranges[i + 1].begin > lastPrimitive)) {
			mid2 = m_Ranges[i].material;
			m_Ranges.Erase(m_Ranges.First() + i, true);
			break;
		}
		if(inside) {
			m_Ranges.Erase(m_Ranges.First() + i, true);
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
		m_Ranges.Insert(MaterialRange(mid1, begin1), m_Ranges.First() + insertAt + 0);
		insertAt++;
	}
	if(insertAt > 0 && m_Ranges[insertAt - 1].material == mid) {
		(void)0;
	} else {
		m_Ranges.Insert(MaterialRange(mid, firstPrimitive), m_Ranges.First() + insertAt);
		++insertAt;
	}

	auto geo = GetGeometry();
	u32 primCount = geo ? geo->GetPrimitiveCount() : 0xFFFFFFFF;
	if(lastPrimitive < primCount) {
		if(insertAt > 0 && m_Ranges[insertAt - 1].material == mid2) {
			(void)0;
		} else {
			m_Ranges.Insert(MaterialRange(mid2, lastPrimitive + 1), m_Ranges.First() + insertAt);
		}
	}

	// Remove unused materials
	core::Array<bool> used;
	used.Resize(m_Materials.Size(), false);
	size_t count = 0;
	for(size_t i = 0; i < m_Ranges.Size(); ++i) {
		if(used[m_Ranges[i].material] == false)
			++count;
		used[m_Ranges[i].material] = true;
	}

	if(count != m_Materials.Size()) {
		core::Array<u32> replaceTable;
		replaceTable.Resize(m_Materials.Size());
		for(size_t i = 0; i < m_Materials.Size(); ++i) {
			if(!used[i]) {
				size_t idx = i;
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

void Mesh::GetMaterialRange(size_t rangeIndex, size_t& materialIndex, size_t& firstPrimitive, size_t& lastPrimitive)
{
	auto geo = GetGeometry();
	u32 primCount = geo ? geo->GetPrimitiveCount() : 0;
	auto& range = m_Ranges.At(rangeIndex);
	materialIndex = range.material;
	firstPrimitive = range.begin;
	lastPrimitive = rangeIndex + 1 < m_Ranges.Size() ? m_Ranges[rangeIndex + 1].begin - 1 : GetGeometry()->GetPrimitiveCount() - 1;
	lastPrimitive = math::Min<u32>(lastPrimitive, primCount);
}

size_t Mesh::GetRangeCount() const
{
	return m_Ranges.Size();
}

core::Name Mesh::GetReferableType() const
{
	return core::ResourceType::Mesh;
}

} // namespace video
} // namespace lux