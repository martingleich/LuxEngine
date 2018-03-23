#ifndef INCLUDED_LUX_SERIAL_SERIALIZER_H
#define INCLUDED_LUX_SERIAL_SERIALIZER_H
#include "serial/StructuralTable.h"
#include "core/Referable.h"
#include "core/VariableAccess.h"
#include "io/ioConstants.h"

namespace lux
{
namespace serial
{

class StructuralTableLink
{
	struct StructLink
	{
		u32 strct;
		bool isSame;
		core::Array<u32> elemMap;
	};

public:
	// Map from src StructuralTable to dst StructuralTable.
	StructuralTableLink(const StructuralTable* srcTable, const StructuralTable* dstTable)
	{
		m_Src = srcTable;
		m_Dst = dstTable;
		if(m_Src == m_Dst)
			return;

		// Create mapping, mapping elements from src to dst
		// For each src structure
		StructLink link;
		for(auto& src : m_Src->GetStructures()) {
			auto srcId = src.typeId;
			auto& srcName = src.name;

			// Find the matching dst structure
			auto dstId = m_Src->GetStructureId(*srcName);
			link.strct = dstId;
			link.isSame = true;
			link.elemMap.Clear();
			if(link.strct) {
				// If the dst structure exists
				auto& srcElems = m_Src->GetStructure(srcId).elements;
				u32 id2 = 1; //
				// For each src element
				for(auto& srcElem : srcElems) {
					// Get the dest element
					auto dstElem = m_Dst->GetStructureElement(dstId, srcElem.name);
					u32 id = 0;
					// If the dst element exists and is of the same type
					if(dstElem && dstElem->type == srcElem.type) {
						// Add it to the list
						id = dstElem->elemId;
					} // else if not matching dst element -> Add 0 to list

					if(id2 != id)
						link.isSame = false;
					++id2;
					link.elemMap.PushBack(id);
				}
			}
			// else if no dst struct -> Add empty map

			m_Mapping.PushBack(std::move(link));
		}
	}

	void Map(u32 srcType, u32& dstType, bool& isSame) const
	{
		if(m_Src == m_Dst) {
			dstType = srcType;
			isSame = true;
		} else {
			auto& map = m_Mapping[srcType];
			dstType = map.strct;
			isSame = map.isSame;
		}
	}

	void Map(
		u32 srcType, u32 srcElem,
		u32& dstType, u32& dstElem) const
	{
		if(m_Src == m_Dst) {
			dstType = srcType;
			dstElem = srcElem;
		} else {
			auto& map = m_Mapping[srcType];
			dstType = map.strct;
			dstElem = map.elemMap[srcElem];
		}
	}

	bool IsSame(u32 srcType) const
	{
		u32 dst;
		bool isSame;
		Map(srcType, dst, isSame);
		return isSame;
	}

private:
	const StructuralTable* m_Src;
	const StructuralTable* m_Dst;

	core::Array<StructLink> m_Mapping;
};

class ObjectMap : public ReferenceCounted
{
public:
	virtual void AddObject(core::ID objectId, u32 fileId) = 0;
	virtual core::ID GetObject(u32 fileId) = 0;
};

class Serializer : public ReferenceCounted
{
public:
	Serializer(const StructuralTable* inClass,
		const StructuralTable* outClass,
		ObjectMap* objectMap = nullptr) :
		m_InClass(inClass),
		m_OutClass(outClass),
		m_ObjectMap(objectMap)
	{
	}

	virtual ~Serializer() {}

	virtual void BeginStructure(u32 sid) = 0;
	virtual void EndStructure() = 0;

	virtual void WriteAll(const void* baseAddr) = 0;
	virtual void ReadAll(void* baseAddr) = 0;

	virtual void WriteElement(const char* element, const void* elemAddr) = 0;
	virtual void ReadElement(const char* element, void* elemAddr) = 0;

	virtual void WriteType(core::Type type, const void* data) = 0;
	virtual void ReadType(core::Type type, void* data) = 0;

	virtual void Seek(u32 move, io::ESeekOrigin origin = io::ESeekOrigin::Cursor) = 0;
	virtual u32 GetCursor() const = 0;

	const StructuralTable* GetInClass() const
	{
		return m_InClass;
	}

	const StructuralTable* GetOutClass() const
	{
		return m_OutClass;
	}

protected:
	const StructuralTable* m_InClass;
	const StructuralTable* m_OutClass;
	StrongRef<ObjectMap> m_ObjectMap;
};

LUX_API StrongRef<Serializer> CreateBinaryMemorySerializer(core::RawMemory& destination, StructuralTable* table, ObjectMap* map);

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SERIAL_SERIALIZER_H