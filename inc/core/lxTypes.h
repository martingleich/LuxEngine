#ifndef INCLUDED_LXTYPES_H
#define INCLUDED_LXTYPES_H
#include "LuxBase.h"

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
		Texture = 0,   //!< A single texturelayer    class: lux::video::MaterialLayer
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
	{}

	//! Construct from type.
	Type(EType t) :
		m_Type(t)
	{}

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

	//! The size of this type.
	/**
	\return The size of type in bytes or 0 if an error occured
	*/
	u32 GetSize() const
	{
		u32 type = GetBaseType().m_Type;
		static const u32 TYPE_SIZES[] = { sizeof(void*) + 1,
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
			0 };

		if (type < (sizeof(TYPE_SIZES) / sizeof(*TYPE_SIZES)))
			return TYPE_SIZES[type];
		else
			assertNeverReach("Unknown type used");
		return 0;
	}

private:
	EType m_Type;
};

//! Available Types for params

//! This class contains information about a type
template <typename T>
class TypeInfo
{
public:
	static const Type typeId; //!< The typeId
};

///\cond INTERNAL
const Type TypeInfo<int>::typeId = Type::Integer;
const Type TypeInfo<u32>::typeId = Type::U32;
const Type TypeInfo<float>::typeId = Type::Float;
const Type TypeInfo<bool>::typeId = Type::Bool;
template <typename T>
const Type TypeInfo<T>::typeId = Type::Unknown;
// Andere typen in der zugehörigen Datei
///\endcond

} // !namespace core
} // !namespace lux
#endif // !INCLUDED_LXTYPES_H
