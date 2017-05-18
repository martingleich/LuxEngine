#ifndef INCLUDED_LXTYPES_H
#define INCLUDED_LXTYPES_H
#include "LuxBase.h"
#include "lxException.h"
#include <string.h>

namespace lux
{
namespace core
{

//! Represent a type in the lux-engine
/**
This class is used to save type information.
*/
class Type
{
public:
	//! The diffrent available types.
	enum EType
	{
		Texture = 0,   //!< A single texturelayer    class: lux::video::TextureLayer
		Integer,           //!< A integer    type: int
		Float,         //!< A floating point decimal    type: float
		Color,         //!< A 32-bit color with A8R8G8B8, class lux::video::Color
		Vector2,          //!< A twodimensional float vector, class lux::math::vector2f
		Vector3,          //!< A threedimensional float vector, class lux::math::vector3f
		ColorF,        //!< A floating-point color, class lux::video::Colorf
		U32,           //!< A unsigned integer    type: lux::u32
		Bool,          //!< A boolean type: bool
		Matrix,        //!< A 4x4 float matrix type: lux::math::matrix4
		Vector2Int,      //!< A twodimensional integer vector, class lux::math::vector2i
		Vector3Int,      //!< A threedimensional integer vector, class lux::math::vector3i
		Unknown,       //!< Unknown type
		Internal_MatrixCol,    //!< A 4x4 float matrix in collum order, do not use
		Internal_Composed,      //!< A type composed of diffrent types, do not use
	};

public:
	//! Default constructor, creates invalid type.
	Type() :
		m_Type(Unknown)
	{
	}

	//! Construct from type.
	Type(EType t) :
		m_Type(t)
	{
	}

	Type(const Type& t) = default;

	//! Assing type
	Type& operator=(EType t)
	{
		m_Type = t;
		return *this;
	}

	Type& operator=(const Type& t) = default;

	//! Access the type enumration
	explicit operator EType() const
	{
		return m_Type;
	}

	//! Equality
	bool operator==(EType other) const
	{
		return GetBaseType().m_Type == other;
	}

	//! Equality
	bool operator==(Type other) const
	{
		return GetBaseType().m_Type == other.GetBaseType().m_Type;
	}

	//! Inequality
	bool operator!=(EType other) const
	{
		return !(*this == other);
	}

	//! Inequality
	bool operator!=(Type other) const
	{
		return !(*this == other);
	}

	//! The this type marked as constant.
	bool IsConstant() const
	{
		return (m_Type & 0x80) != 0;
	}

	//! Gives a types based on this one, which is constant.
	Type GetConstantType() const
	{
		return Type((EType)(m_Type | 0x80));
	}

	//! Gives a type based on this, with all modification i.e. constness removed.
	Type GetBaseType() const
	{
		return Type((EType)(m_Type & 0x7F));
	}

	static const u32 TEXTURE_LAYER_SIZE = (sizeof(void*) + 8);

	//! The size of this type.
	/**
	\return The size of type in bytes or 0 if an error occured
	*/
	u32 GetSize() const
	{
		u32 type = GetBaseType().m_Type;
		static const u32 TYPE_SIZES[] = {
			TEXTURE_LAYER_SIZE,
			4,
			4,
			4,
			2 * 4,
			3 * 4,
			4 * 4,
			4,
			1,
			4 * 4 * 4,
			4 * 4,
			4 * 4,
			0,
			4 * 4 * 4,
			0};

		if(type >= (sizeof(TYPE_SIZES) / sizeof(*TYPE_SIZES)))
			throw Exception("Unknown type used");

		return TYPE_SIZES[type];
	}

private:
	EType m_Type;
};

//! Exception thrown when a type mismatch occures.
/**
Some type is not valid in this place, or two types are not compatible.
*/
struct TypeException : ErrorException
{
	explicit TypeException(const char* _msg, Type _typeA = Type::Unknown, Type _typeB = Type::Unknown) :
		ErrorException("type error"),
		msg(_msg),
		typeA(_typeA),
		typeB(_typeB)
	{
	}

	ExceptionSafeString msg;

	//! First type which was part of the problem
	Type typeA;

	//! Second type which was part of the porblem
	Type typeB;
};

//! Available Types for params
template <typename T>
Type GetTypeInfo() { return Type::Unknown; }

template <> inline Type GetTypeInfo<int>() { return Type::Integer; }
template <> inline Type GetTypeInfo<u32>() { return Type::U32; }
template <> inline Type GetTypeInfo<float>() { return Type::Float; }
template <> inline Type GetTypeInfo<bool>() { return Type::Bool; }

//! Converts between base types.
/**
The base types are int, u32, float and bool.
*/
inline bool ConvertBaseType(Type fromType, const void* fromData, Type toType, void* toData)
{
	if(fromType == toType) {
		memcpy(toData, fromData, fromType.GetSize());
		return true;
	}

	if(fromType == Type::Integer) {
		if(toType == Type::U32) {
			*((u32*)toData) = *((int*)fromData);
			return true;
		}
		if(toType == Type::Float) {
			*((float*)toData) = (float)*((int*)fromData);
			return true;
		}
		if(toType == Type::Bool) {
			*((bool*)toData) = *((int*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Type::U32) {
		if(toType == Type::Integer) {
			*((int*)toData) = *((u32*)fromData);
			return true;
		}
		if(toType == Type::Float) {
			*((float*)toData) = (float)*((u32*)fromData);
			return true;
		}

		if(toType == Type::Bool) {
			*((bool*)toData) = *((u32*)fromData) ? true : false;
			return true;
		}
		return false;
	}

	if(fromType == Type::Float) {
		if(toType == Type::Integer) {
			*((int*)toData) = (int)*((float*)fromData);
			return true;
		}
		if(toType == Type::U32) {
			*((u32*)toData) = (u32)*((float*)fromData);
			return true;
		}
		return false;
	}

	if(fromType == Type::Bool) {
		if(toType == Type::Integer) {
			*((int*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Type::U32) {
			*((u32*)toData) = *((bool*)fromData) ? 1 : 0;
			return true;
		}
		if(toType == Type::Float) {
			*((float*)toData) = *((bool*)fromData) ? 1.0f : 0.0f;
			return true;
		}
		return false;
	}

	return false;
}

inline bool IsConvertible(Type fromType, Type toType)
{
	if(fromType == toType) {
		return true;
	}

	if(fromType == Type::Integer) {
		if(toType == Type::U32) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}
		if(toType == Type::Bool) {
			return true;
		}
		return false;
	}

	if(fromType == Type::U32) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}

		if(toType == Type::Bool) {
			return true;
		}
		return false;
	}

	if(fromType == Type::Float) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::U32) {
			return true;
		}
		return false;
	}

	if(fromType == Type::Bool) {
		if(toType == Type::Integer) {
			return true;
		}
		if(toType == Type::U32) {
			return true;
		}
		if(toType == Type::Float) {
			return true;
		}
		return false;
	}

	return false;
}

} // !namespace core
} // !namespace lux

#endif // !INCLUDED_LXTYPES_H
