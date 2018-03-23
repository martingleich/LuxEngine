#ifndef INCLUDED_LUX_SERIAL_STRUCTURE_H
#define INCLUDED_LUX_SERIAL_STRUCTURE_H
#include "core/lxString.h"
#include "core/lxArray.h"
#include "core/lxID.h"

#include "core/Referable.h"
#include "math/Vector3.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "video/Color.h"

namespace lux
{

namespace serial
{

//! Description of a single element in a structure
struct StructureElement
{
	//! Name of the element
	core::SharedString name;

	//! Core type of the elememnt
	core::Type type;

	//! The index of the element in the structure.
	u32 elemId;

	//! Offset of the element from the start of the structure in bytes.
	u32 offset;

	//! Offset of the mapped element.
	/**
	Distance from the mapped element to (Serialzable*)this in bytes.
	Only available if hasObjectData is true.
	*/
	u32 objectOffset;

	//! The size of the element if serialized.
	/**
	May be diffrent from type.GetSize().
	But will always be the same for trivial types.
	*/
	u32 size;

	//! Has this element object information
	bool hasObjectData;
};

//! Description of a structure.
struct Structure
{
	//! Name of the type
	core::SharedString name;

	//! Id of the type
	u32 typeId;

	//! Version id of the type.
	u32 version;

	//! Size if serialized in bytes
	u32 size;

	//! Can the whole structure be copied at once via memcpy
	bool isCompact;

	//! Elements of the structure.
	core::Array<StructureElement> elements;
};

#define LX_STRUCTURE_BUILDER_ADD(tname, type) \
void Add##tname(const core::String& name) \
{ \
	AddElement(core::Types::##tname(), name, nullptr); \
} \
void Add##tname(const core::String& name, const type& addr) \
{ \
	AddElement(core::Types::##tname(), name, &addr); \
}

class StructuralTable;

//! Helper class to build a structure.
class StructureBuilder
{
public:
	//! Constructor
	/**
	\param structTable The struct table where this element will be placed.
	\param base The pointer to the mapped class.
	\param name The name of the structure.
	\param version The version number of the structure.
	*/
	StructureBuilder(
		StructuralTable* structTable,
		const void* base,
		const core::String& name,
		u32 version);

	//! Add a single element, type is inferred from template.
	LUX_API void AddElement(core::Type type, const core::String& name, const void* ptr);

	template <typename T>
	void AddElement(const core::String& name, const T& addr)
	{
		AddElement(core::TemplType<T>::Get(), name, &addr);
	}
	template <typename T>
	void AddElement(const core::String& name)
	{
		AddElement(core::TemplType<T>::Get(), name);
	}

	LX_STRUCTURE_BUILDER_ADD(Integer, s32);
	LX_STRUCTURE_BUILDER_ADD(Float, float);
	LX_STRUCTURE_BUILDER_ADD(Vector2F, math::Vector2F);
	LX_STRUCTURE_BUILDER_ADD(Vector3F, math::Vector3F);
	LX_STRUCTURE_BUILDER_ADD(ColorF, video::ColorF);
	LX_STRUCTURE_BUILDER_ADD(Color, video::Color);
	LX_STRUCTURE_BUILDER_ADD(QuaternionF, math::QuaternionF);
	LX_STRUCTURE_BUILDER_ADD(Boolean, bool);
	LX_STRUCTURE_BUILDER_ADD(String, core::String);
	LX_STRUCTURE_BUILDER_ADD(StrongID, core::ID);
	LX_STRUCTURE_BUILDER_ADD(WeakID, core::ID);

	//! Finish the creation of the structure.
	/**
	The finished structure is placed in the structural table.
	\return The type id of the new structure.
	*/
	LUX_API u32 Finalize();

private:
	StructuralTable* m_StructTable;
	const void* m_BasePtr;

	Structure m_Structure;

	u32 m_NextOffset;
	u32 m_NextElemId;
};

#undef LX_STRUCTURE_BUILDER_ADD

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SERIAL_STRUCTURE_H