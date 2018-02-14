#ifndef INCLUDED_SERIAL_STRUCTURE_H
#define INCLUDED_SERIAL_STRUCTURE_H
#include "core/lxString.h"
#include "core/lxArray.h"
#include "core/lxID.h"

#include "math/Vector3.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "video/Color.h"

namespace lux
{

namespace serial
{

//! Basetypes of the serialization system.
/**
Value of the array is identical to type id.
*/
enum EType
{
	Type_Invalid = 0,
	Type_Int,
	Type_Float,
	Type_Vec2,
	Type_Vec3,
	Type_Colorf,
	Type_Color,
	Type_Quaternion,
	Type_Bool,

	Type_String,
	Type_Blob,
	Type_StrongRef,
	Type_WeakRef,
};

//! Retrieve the core::Type based on a base type.
LUX_API core::Type GetCoreType(u32 baseType);
//! Retrieve the basetype from a core::Type
LUX_API u32 GetFromCoreType(core::Type type);

//! Description of a type
struct Type
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

	//! Is the type a base type, structure otherwise
	bool isStructure;
};

//! Description of a single element in a structure
struct StructureElement
{
	//! Name of the element
	core::SharedString name;

	//! Type of the elememt.
	u32 typeId;

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

	//! Is this element an array.
	bool isArray;

	//! Has this element object information
	bool hasObjectData;
};

//! Description of a structure.
struct Structure : public Type
{
	//! Elements of the structure.
	core::Array<StructureElement> elements;
};

#define LX_STRUCTURE_BUILDER_ADD(tname, type) \
void Add##tname(const core::String& name) \
{ \
	AddBaseType(Type_##tname, name, nullptr); \
} \
void Add##tname(const core::String& name, const type& addr) \
{ \
	AddBaseType(Type_##tname, name, &addr); \
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
	template <typename T>
	void AddType(const core::String& name, const T& addr)
	{
		AddBaseType(
			GetFromCoreType(core::TemplType<T>::Get()),
			name, &addr);
	}

	LX_STRUCTURE_BUILDER_ADD(Int, s32);
	LX_STRUCTURE_BUILDER_ADD(Float, float);
	LX_STRUCTURE_BUILDER_ADD(Vec2, math::Vector2F);
	LX_STRUCTURE_BUILDER_ADD(Vec3, math::Vector3F);
	LX_STRUCTURE_BUILDER_ADD(Colorf, video::Colorf);
	LX_STRUCTURE_BUILDER_ADD(Color, video::Color);
	LX_STRUCTURE_BUILDER_ADD(Quaternion, math::QuaternionF);
	LX_STRUCTURE_BUILDER_ADD(Bool, bool);
	LX_STRUCTURE_BUILDER_ADD(String, core::String);
	LX_STRUCTURE_BUILDER_ADD(StrongRef, core::ID);
	LX_STRUCTURE_BUILDER_ADD(WeakRef, core::ID);

	//! Add a blob element.
	void AddBlob(const core::String& name)
	{
		AddBaseType(Type_Blob, name, nullptr);
	}

	//! Add a array element, type is inferred from template.
	template <typename T>
	void AddArray(const core::String& name)
	{
		AddArrayBaseType(GetFromCoreType(core::TemplType<T>::Get()), name, nullptr);
	}

	//! Add a array element, type is inferred from template.
	template <typename T>
	void AddArray(const core::String& name, const core::Array<T>& addr)
	{
		AddArrayBaseType(GetFromCoreType(core::TemplType<T>::Get()), name, (const core::ArrayAlloc*)&addr);
	}

	//! Finish the creation of the structure.
	/**
	The finished structure is placed in the structural table.
	\return The type id of the new structure.
	*/
	LUX_API u32 Finalize();

private:
	LUX_API void AddBaseType(u32 type, const core::String& name, const void* ptr);
	LUX_API void AddArrayBaseType(u32 type, const core::String& name, const void* ptr);
	void AddType(u32 type, u32 size, bool array, const core::String& name, const void* ptr);

private:
	StructuralTable* m_StructTable;
	const void* m_BasePtr;

	Structure m_Data;

	u32 m_NextOffset;
};

#undef LX_STRUCTURE_BUILDER_ADD

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_STRUCTURE_H