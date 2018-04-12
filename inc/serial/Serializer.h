#ifndef INCLUDED_LUX_SERIAL_SERIALIZER_H
#define INCLUDED_LUX_SERIAL_SERIALIZER_H
#include "core/ReferenceCounted.h"
#include "core/VariableAccess.h"
#include "io/ioConstants.h"
#include "serial/StructuralTable.h"

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
	virtual u32 OnIdWrite(core::ID objectId) = 0;
	virtual core::ID OnIdRead(u32 fileId) = 0;
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

	//! Begins writing a structure.
	/**
	The structure is written at the current cursor position.
	Calls to BeginStructure can't be nested, meaning BeginStructure can't be called before the previous Begin didn't end.
	\param sid The structure index in the inClass
	*/
	virtual void BeginStructure(u32 sid) = 0;
	//! Ends writing a structure.
	/**
	Must be called after each BeginStructure.
	Places the cursor at the first byte after the written structure.
	*/
	virtual void EndStructure() = 0;

	//! Write the complete structure
	/**
	If this function is called more then once, no additional data is written.
	\param baseAddr The base addr given when creating the structure.
	*/
	virtual void WriteAll(const void* baseAddr) = 0;
	//! Reads the complete structure
	/**
	If this function is called more then once, the data is not reread.
	\param baseAddr The base addr given when creating the structure.
	*/
	virtual void ReadAll(void* baseAddr) = 0;

	//! Write a single element of the structure.
	virtual void WriteElement(const char* element, const void* elemAddr) = 0;
	//! Write a single element of the structure.
	virtual void ReadElement(const char* element, void* elemAddr) = 0;

	//! Write a single object of some type.
	virtual void WriteType(core::Type type, const void* data) = 0;
	//! Read a single object of some type.
	virtual void ReadType(core::Type type, void* data) = 0;

	//! Moves the cursor.
	virtual void Seek(u32 move, io::ESeekOrigin origin = io::ESeekOrigin::Cursor) = 0;
	//! Gets the cursor position.
	virtual u32 GetCursor() const = 0;

	//! Get the structural table for the in-memory representation.
	const StructuralTable* GetInClass() const
	{
		return m_InClass;
	}

	//! Get the structural table for the outoff-memory representation(i.e. In file)
	const StructuralTable* GetOutClass() const
	{
		return m_OutClass;
	}

protected:
	const StructuralTable* m_InClass;
	const StructuralTable* m_OutClass;
	StrongRef<ObjectMap> m_ObjectMap;
};

class Serializable : public virtual ReferenceCounted
{
public:
	virtual void Serialize(serial::Serializer* target) = 0;
	virtual void DeSerialize(serial::Serializer* source) = 0;
	virtual void InitSerializer(serial::StructureBuilder& builder) const = 0;
	virtual u32 GetSerializerStructure() const = 0;
};

LUX_API StrongRef<Serializer> CreateBinaryMemorySerializer(core::RawMemory& destination, StructuralTable* table, ObjectMap* map);

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SERIAL_SERIALIZER_H