#ifndef INCLUDED_LUX_VERTEX_FORMAT_H
#define INCLUDED_LUX_VERTEX_FORMAT_H
#include "core/lxArray.h"
#include "core/lxString.h"
#include "math/lxMath.h"

namespace lux
{
namespace video
{

//! A single element of a vertex
struct VertexElement
{
	//! The type of a vertex element
	enum class EType
	{
		Float1, //!< 1 float saved as (Value, 0, 0, 1) in the register.
		Float2, //!< 2 floats saved as (Value, Value, 0, 1) in the register.
		Float3, //!< 3 floats saved as (Value, Value, Value, 1) in the register.
		Float4, //!< 4 floats.
		Color, //!< 32-Bit-Color.
		Byte4, //!< 4 single bytes.
		Short2, //!< 2 signed 16Bit ints saved as (Value, Value, 0, 1) in the register.
		Short4, //!< 4 signed 16Bit ints.
		Unknown, //!< Unknown type.
	};

	//! Usage of a elemennt.
	enum class EUsage
	{
		Position = 0, //!< D3D/GL: POSITION
		PositionNT, //!< D3D/GL: POSITIONNT
		Normal, //!< D3D/GL: NORMAL
		Tangent, //!< D3D/GL: TANGENT
		Binormal, //!< D3D/GL: BINORMAL
		Texcoord0, //!< D3D/GL: TEXCOORD0 
		Texcoord1, //!< D3D/GL: TEXCOORD1
		Texcoord2, //!< D3D/GL: TEXCOORD2
		Texcoord3, //!< D3D/GL: TEXCOORD3
		Diffuse, //!< D3D: COLOR0 GL: DIFFUSE
		Specular, //!< D3D: COLOR1 GL: SPECULAR
		BlendWeight, //!< D3D/GL: BLENDWEIGHT
		BlendIndices, //!< D3D/GL: BLENDINDICES
		Sample, //!< D3D/GL: SAMPLE
		Unknown //!< Unknown usage.
	};

	//! Get the usage for the i-th Texturecoordinate
	static EUsage TexcoordN(int i)
	{
		lxAssert(i >= 0 && i < 4);
		return EUsage((int)EUsage::Texcoord0 + i);
	}

	static int GetTypeSize(VertexElement::EType type)
	{
		switch(type) {
		case VertexElement::EType::Float1:
			return 4;
		case VertexElement::EType::Float2:
			return 8;
		case VertexElement::EType::Float3:
			return 12;
		case VertexElement::EType::Float4:
			return 16;
		case VertexElement::EType::Color:
			return 4;
		case VertexElement::EType::Byte4:
			return 4;
		case VertexElement::EType::Short2:
			return 4;
		case VertexElement::EType::Short4:
			return 8;
		case VertexElement::EType::Unknown:
			return 0;
		}
		return 0;
	}

	static VertexElement::EType GetSematicDefaultType(VertexElement::EUsage usage)
	{
		switch(usage) {
		case VertexElement::EUsage::Position:
			return VertexElement::EType::Float4;
		case VertexElement::EUsage::PositionNT:
			return VertexElement::EType::Float4;
		case VertexElement::EUsage::Normal:
			return VertexElement::EType::Float3;
		case VertexElement::EUsage::Tangent:
			return VertexElement::EType::Float3;

		case VertexElement::EUsage::Binormal:
			return VertexElement::EType::Float3;
		case VertexElement::EUsage::Texcoord0:
			return VertexElement::EType::Float2;
		case VertexElement::EUsage::Texcoord1:
			return VertexElement::EType::Float2;
		case VertexElement::EUsage::Texcoord2:
			return VertexElement::EType::Float2;
		case VertexElement::EUsage::Texcoord3:
			return VertexElement::EType::Float2;
		case VertexElement::EUsage::Diffuse:
			return VertexElement::EType::Color;
		case VertexElement::EUsage::Specular:
			return VertexElement::EType::Color;
		case VertexElement::EUsage::BlendWeight:
			return VertexElement::EType::Byte4;
		case VertexElement::EUsage::BlendIndices:
			return VertexElement::EType::Byte4;
		case VertexElement::EUsage::Sample:
			return VertexElement::EType::Byte4;
		case VertexElement::EUsage::Unknown:
			return VertexElement::EType::Unknown;
		}
		return VertexElement::EType::Unknown;
	}

	//! Get the invalid vertex element.
	static VertexElement Invalid()
	{
		return VertexElement();
	}

	//! Initialize an invalid element.
	VertexElement() :
		m_Offset(0),
		m_Type(EType::Unknown),
		m_Usage(EUsage::Unknown)
	{
	}

	//! Initalize by members
	VertexElement(int _offset, EType _type, EUsage _sematic) :
		m_Offset(_offset),
		m_Type(_type),
		m_Usage(_sematic)
	{
	}

	//! Check if a element is valid.
	bool IsValid() const
	{
		return (m_Type != EType::Unknown && m_Usage != EUsage::Unknown);
	}

	bool operator==(const VertexElement& other) const
	{
		return
			m_Offset == other.m_Offset &&
			m_Type == other.m_Type &&
			m_Usage == other.m_Usage;
	}
	bool operator!=(const VertexElement& other) const
	{
		return !(*this == other);
	}

	EType GetType() const { return m_Type; }
	EUsage GetUsage() const { return m_Usage; }
	int GetSize() const { return GetTypeSize(m_Type); }
	int GetOffset() const { return m_Offset; }

private:
	//! Offset of this element zu begin of element.
	int m_Offset;
	//! Type of the element.
	EType m_Type;
	//! Usage of the element.
	EUsage m_Usage;
};

} // namespace video

namespace core
{
template <>
struct HashType<video::VertexElement>
{
	int operator()(const video::VertexElement& e) const
	{
		if(!e.IsValid())
			return 0;

		int out = 7;
		out += 31 * out + e.GetOffset();
		out += 31 * out + (int)e.GetUsage();
		out += 31 * out + (int)e.GetType();
		return out;
	}
};
} // namespace core

namespace video
{

//! The format a of a vertex.
class VertexFormat
{
public:
	//! Standard 3D vertexformat.
	/**
	Identical to: \ref Vertex3D
	*/
	LUX_API static const VertexFormat STANDARD;
	//! Transformed vertexformat.
	/**
	Identical to: \ref VertexTransformed
	*/
	LUX_API static const VertexFormat TRANSFORMED;
	//! Position only vertexformat
	/**
	Identical to: \ref VertexPosOnly
	*/
	LUX_API static const VertexFormat POS_ONLY;
	//! Position only with w coordiante vertexformat
	/**
	Identical to: \ref VertexPosWOnly
	*/
	LUX_API static const VertexFormat POSW_ONLY;
	//! Two texturecoordinate vertexformat
	/**
	Identical to: \ref Vertex2TCoords
	*/
	LUX_API static const VertexFormat TWO_TEXTURE;
	//! Tangent and binormal vertexformat
	/**
	Identical to: \ref VertexTangents
	*/
	LUX_API static const VertexFormat TANGENTS;
	//! 3D texturecoordinate vertexformat
	/**
	Identical to: \ref Vertex3DTCoord
	*/
	LUX_API static const VertexFormat TEXTURE_3D;
	//! Standard 2D vertexformat
	/**
	Identical to: \ref Vertex2D
	*/
	LUX_API static const VertexFormat STANDARD_2D;

private:
	struct SharedData : public ReferenceCounted
	{
		core::String name;
		core::Array<VertexElement> elements;
		int stride;
		int hash;

		SharedData(core::StringView name, std::initializer_list<VertexElement> elems, int _stride)
		{
			name = name;
			if(elems.size() != 0) {
				elements = elems;
				struct SortByOffset
				{
					bool Smaller(const VertexElement& a, const VertexElement& b) const
					{
						return a.GetOffset() < b.GetOffset();
					}

					bool Equal(const VertexElement& a, const VertexElement& b) const
					{
						return a.GetOffset() == b.GetOffset();
					}
				};

				core::Sort(elements, SortByOffset());
				stride = elements.Back().GetOffset() + elements.Back().GetSize();
			} else {
				stride = 0;
			}
			if(stride < _stride)
				stride = _stride;

			hash = 7;
			core::HashType<VertexElement> hasher;
			for(int i = 0; i < elements.Size(); ++i)
				hash = 31 * hash + hasher(elements[i]);
		}
	};

public:
	//! Create an empty format.
	LUX_API VertexFormat();

	//! Create a format from name elements and stride
	/*
	\param stride If passed stride is bigger than the calculated stride, padding is added. Otherwise the calculated stride is used.
	*/
	VertexFormat(core::StringView name, std::initializer_list<VertexElement> elems, int stride = 0) :
		m_Data(LUX_NEW(SharedData)(name, elems, stride))
	{
	}

	bool operator==(const VertexFormat& other) const
	{
		if(m_Data == other.m_Data)
			return true;
		if(!m_Data || !other.m_Data)
			return false;
		if(m_Data->hash != other.m_Data->hash)
			return false;

		if(GetElemCount() != other.GetElemCount())
			return false;

		for(int i = 0; i < GetElemCount(); ++i) {
			if(GetElement(i) != other.GetElement(i))
				return false;
		}

		return true;
	}

	bool operator!=(const VertexFormat& other) const
	{
		return !(*this == other);
	}

	const core::String& GetName() const
	{
		return m_Data->name;
	}

	int GetElemCount() const
	{
		return m_Data->elements.Size();
	}

	VertexElement GetElement(int elem) const
	{
		return m_Data->elements.At(elem);
	}

	int GetStride() const
	{
		return m_Data->stride;
	}

	//! Get an element by stream and element usage.
	/**
	Returns an invalid element if the stream doesn't exist.
	*/
	VertexElement GetElement(VertexElement::EUsage usage) const
	{
		for(int elemId = 0; elemId < GetElemCount(); ++elemId) {
			VertexElement e = GetElement(elemId);
			if(e.GetUsage() == usage)
				return e;
		}

		return VertexElement::Invalid();
	}

	//! Gets a hash for this element
	int GetHash() const
	{
		return m_Data->hash;
	}

private:
	StrongRef<SharedData> m_Data;
};

//! The declaration of a vertex type
class VertexFormatBuilder
{
public:
	//! Create a declaration with a single empty stream.
	VertexFormatBuilder()
	{
		Clear();
	}

	//! Reset a declaration to a single empty stream.
	void Clear()
	{
		m_Elements.Clear();
		m_Stride = 0;
	}

	//! Add an element.
	/**
	\param usage The usage of the new element
	\param type The type of the new element,
		pass Unknown to select the default type for this usage.
	*/
	void AddElement(VertexElement::EUsage usage,
		VertexElement::EType type = VertexElement::EType::Unknown)
	{
		AddElement(m_Stride, usage, type);
	}

	//! Add an element to the current stream.
	/**
	\param offset Offset of the element to the begin of the stream in bytes.
		Should not collide with already created elements.
		If there is free space between the previous element and this one, padding is added.
	\param usage The usage of the new element
	\param type The type of the new element,
		pass Unknown to select the default type for this usage.
	*/
	void AddElement(int offset,
		VertexElement::EUsage usage,
		VertexElement::EType type = VertexElement::EType::Unknown)
	{
		if(usage == VertexElement::EUsage::Unknown)
			throw core::GenericInvalidArgumentException("usage", "Invalid vertex element Usage");
		if(offset < 0)
			throw core::GenericInvalidArgumentException("usage", "Invalid vertex element Usage");
		if(type == VertexElement::EType::Unknown)
			type = VertexElement::GetSematicDefaultType(usage);

		m_Elements.EmplaceBack(offset, type, usage);
		m_Stride = math::Max(m_Stride, offset) + VertexElement::GetTypeSize(type);
	}

	VertexFormat Build(core::StringView name, int stride = 0)
	{
		return VertexFormat(name,
			std::initializer_list<VertexElement>(m_Elements.Data(), m_Elements.Data() + m_Elements.Size()),
			stride);
	}

private:
	core::Array<VertexElement> m_Elements;
	int m_Stride;
};

} // namespace video

namespace core
{
template <>
struct HashType<video::VertexFormat>
{
	int operator()(const video::VertexFormat& e) const
	{
		return e.GetHash();
	}
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_VERTEX_FORMAT_H