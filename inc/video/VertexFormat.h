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

	//! Stream id of this element.
	int stream;
	//! Offset of this element zu begin of stream.
	int offset;
	//! Type of the element.
	EType type;
	//! Usage of the element.
	EUsage sematic;

	//! Get the invalid vertex element.
	static VertexElement Invalid()
	{
		return VertexElement();
	}

	//! Initialize an invalid element.
	VertexElement() :
		stream(-1),
		offset(0),
		type(EType::Unknown),
		sematic(EUsage::Unknown)
	{
	}

	//! Initalize by members
	VertexElement(int _stream, int _offset, EType _type, EUsage _sematic) :
		stream(_stream),
		offset(_offset),
		type(_type),
		sematic(_sematic)
	{
	}

	//! Check if a element is valid.
	bool IsValid() const
	{
		return (type != EType::Unknown && sematic != EUsage::Unknown);
	}

	bool operator==(const VertexElement& other) const
	{
		return 
			stream == other.stream &&
			offset == other.offset &&
			type == other.type &&
			sematic == other.sematic;
	}

	bool operator!=(const VertexElement& other) const
	{
		return !(*this == other);
	}
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
		out += 31 * out + e.stream;
		out += 31 * out + e.offset;
		out += 31 * out + (int)e.sematic;
		out += 31 * out + (int)e.type;
		return out;
	}
};
} // namespace core

namespace video
{

//! The declaration of a vertex type
class VertexDeclaration
{
public:
	//! Create a declaration with a single empty stream.
	VertexDeclaration() :
		m_CurrentStream(nullptr),
		m_IsNormalized(true)
	{
		AddStream();
	}

	//! Reset a declaration to a single empty stream.
	void Clear()
	{
		m_Data.Clear();
		m_Streams.Clear();

		m_CurrentStream = nullptr;

		m_IsNormalized = true;
		AddStream();
	}

	//! Add a new stream
	/**
	Finishes the last stream, and selects a new one.
	*/
	void AddStream()
	{
		int firstElem = m_Data.IsEmpty() ? 0 : (m_Data.Size() - 1);
		m_Streams.PushBack(Stream(firstElem));

		m_CurrentStream = &m_Streams[m_Streams.Size() - 1];
		m_IsNormalized = false;
	}

	//! Add an element to the current stream.
	/**
	\param usage The usage of the new element
	\param type The type of the new element,
		pass Unknown to select the default type for this usage.
	*/
	void AddElement(VertexElement::EUsage usage,
		VertexElement::EType type = VertexElement::EType::Unknown)
	{
		if(type == VertexElement::EType::Unknown)
			type = GetSematicDefaultType(usage);

		int offset;
		if(m_CurrentStream->elemCount == 0)
			offset = 0;
		else
			offset = m_CurrentStream->stride;

		m_Data.PushBack(Element(offset, usage, type));
		m_CurrentStream->stride = math::Max(m_CurrentStream->stride, offset) + GetTypeSize(type);
		m_CurrentStream->elemCount += 1;

		m_IsNormalized = false;
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
		if(type == VertexElement::EType::Unknown)
			type = GetSematicDefaultType(usage);

		m_Data.PushBack(Element(offset, usage, type));
		m_CurrentStream->stride = math::Max(m_CurrentStream->stride, offset) + GetTypeSize(type);
		m_CurrentStream->elemCount += 1;

		m_IsNormalized = false;
	}

	//! Get the total number of element in the format.
	int GetElemCount() const
	{
		return m_Data.Size();
	}

	//! Get the number of element in a given stream.
	/**
	Returns 0 if stream does not exist.
	*/
	int GetElemCount(int stream) const
	{
		if(stream < m_Streams.Size())
			return m_Streams[stream].elemCount;
		else
			return 0;
	}

	//! Get the number of streams.
	int GetStreamCount() const
	{
		return m_Streams.Size();
	}

	//! Get some element in the declaraction
	/**
	\param stream Stream to look into.
	\param elem The id of the element in the stream
	\return Copy of the vertexelement, or the invalid element if requested
		element does not exist.
	*/
	VertexElement GetElement(int stream, int elem) const
	{
		if(stream >= m_Streams.Size())
			return VertexElement::Invalid();

		int firstElem = m_Streams[stream].firstElement;
		if(firstElem >= m_Streams.Size())
			return VertexElement::Invalid();

		if(elem >= m_Streams[stream].elemCount)
			return VertexElement::Invalid();

		Element e = m_Data[firstElem + elem];
		return VertexElement((u8)stream, (u16)e.offset, e.type, e.usage);
	}

	//! Get the stride of a given stream.
	/**
	\return Number of bytes for as single element of the stream,
		or 0 if stream doesn't exist.
	*/
	int GetStride(int stream = 0) const
	{
		if(stream < m_Streams.Size())
			return m_Streams[stream].stride;
		else
			return 0;
	}

	//! Checks if a declaration is valid.
	bool IsValid() const
	{
		if(m_Streams.Size() == 0)
			return false;

		for(int streamID = 0; streamID < m_Streams.Size(); ++streamID) {
			const Stream& stream = m_Streams[streamID];
			if(stream.elemCount == 0)
				return false;
			for(int elemID = 0; elemID < stream.elemCount; ++elemID) {
				const Element& elem = m_Data[stream.firstElement + elemID];
				if(elem.type == VertexElement::EType::Unknown)
					return false;
				if(elem.usage == VertexElement::EUsage::Unknown)
					return false;
			}
		}

		return true;
	}

	//! Normalizes the representation of the declaration.
	/**
	The normalized declaration describes the same vertex declaration, but
	all elements are sorted by their offset.
	*/
	void Normalize()
	{
		if(m_IsNormalized)
			return;

		struct SortByOffset
		{
			bool Smaller(const Element& a, const Element& b) const
			{
				return a.offset < b.offset;
			}

			bool Equal(const Element& a, const Element& b) const
			{
				return a.offset == b.offset;
			}
		};

		for(int streamId = m_Streams.Size(); streamId < m_Streams.Size(); ++streamId) {
			const auto& stream = m_Streams[streamId];

			core::Sort(
				core::SliceRangeCount(m_Data, stream.firstElement, stream.elemCount),
				SortByOffset());
		}

		m_IsNormalized = true;
	}

	//! Is the declaration normalized.
	bool IsNormalized() const
	{
		return m_IsNormalized;
	}

private:
	static u16 GetTypeSize(VertexElement::EType type)
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
		default:
			return 0;
		}
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
		default:
			return VertexElement::EType::Unknown;
		}
	}

private:
	struct Element
	{
	public:
		Element() :
			offset(0),
			usage(VertexElement::EUsage::Unknown),
			type(VertexElement::EType::Unknown)
		{
		}

		Element(int _offset, VertexElement::EUsage _usage, VertexElement::EType _type) :
			offset(_offset),
			usage(_usage),
			type(_type)
		{
		}

		int offset;
		VertexElement::EUsage usage;
		VertexElement::EType type;
	};

	struct Stream
	{
		Stream() :
			elemCount(0),
			stride(0),
			firstElement(0)
		{
		}

		Stream(int firstElem) :
			elemCount(0),
			stride(0),
			firstElement(firstElem)
		{
		}

		int elemCount;
		int stride;

		int firstElement;
	};

private:
	core::Array<Stream> m_Streams;
	core::Array<Element> m_Data;

	Stream* m_CurrentStream;

	bool m_IsNormalized;
};

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

public:
	//! Create an empty format.
	VertexFormat() :
		m_Declaration(nullptr),
		m_IsValid(false),
		m_Hash(0)
	{
	}

	//! Create a format from a declaration with some name.
	/**
	Names of vertexformat don't have to be unique, but it always better when they
	are.
	*/
	VertexFormat(const core::String& name, const VertexDeclaration& decl) :
		m_Name(name)
	{
		m_Declaration = std::make_shared<VertexDeclaration>(decl);
		m_Declaration->Normalize();
		m_IsValid = m_Declaration->IsValid();
		m_Hash = CalcHash();
	}

	bool operator==(const VertexFormat& other) const
	{
		if(m_Hash != other.m_Hash)
			return false;

		if(m_Declaration == other.m_Declaration)
			return true;
		if(m_Declaration == nullptr || other.m_Declaration == nullptr)
			return false;

		lxAssert(m_Declaration->IsNormalized());
		if(GetStreamCount() != other.GetStreamCount())
			return false;
		if(GetElemCount() != other.GetElemCount())
			return false;

		for(int streamId = 0; streamId < GetStreamCount(); ++streamId) {
			for(int elemId = 0; elemId < GetElemCount(streamId); ++elemId) {
				if(GetElement(streamId, elemId) !=
					other.GetElement(streamId, elemId))
					return false;
			}
		}

		return true;
	}

	bool operator!=(const VertexFormat& other) const
	{
		return !(*this == other);
	}

	//! Get the name of the format.
	const core::String& GetName() const
	{
		return m_Name;
	}

	//! Get the number of elements in the format.
	int GetElemCount() const
	{
		if(IsValid())
			return m_Declaration->GetElemCount();
		else
			return 0;
	}

	//! Get the number of elements in one stream.
	/**
	Returns 0 if the stream doesn't exist.
	*/
	int GetElemCount(int stream) const
	{
		if(IsValid())
			return m_Declaration->GetElemCount(stream);
		else
			return 0;
	}

	//! Get the number of streams.
	int GetStreamCount() const
	{
		if(IsValid())
			return m_Declaration->GetStreamCount();
		else
			return 0;
	}

	//! Get an element by stream and element id.
	/**
	Returns an invalid element if the stream doesn't exist.
	*/
	VertexElement GetElement(int stream, int elem) const
	{
		if(IsValid())
			return m_Declaration->GetElement(stream, elem);
		else
			return VertexElement::Invalid();
	}

	//! Get the stride of a single stream.
	int GetStride(int stream = 0) const
	{
		if(IsValid())
			return m_Declaration->GetStride(stream);
		else
			return 0;
	}

	//! Checks if the format is valid.
	bool IsValid() const
	{
		return m_Declaration && m_IsValid;
	}

	//! Get an element by stream and element usage.
	/**
	Returns an invalid element if the stream doesn't exist.
	*/
	VertexElement GetElement(int stream, VertexElement::EUsage usage) const
	{
		if(!IsValid() || stream >= GetStreamCount())
			return VertexElement::Invalid();

		for(int elemId = 0; elemId < GetElemCount(stream); ++elemId) {
			VertexElement e = GetElement(stream, elemId);
			if(e.IsValid() && e.sematic == usage)
				return e;
		}

		return VertexElement::Invalid();
	}

	//! Get an element by its usage.
	/**
	Get the first element in the first stream with the given usage.
	Returns an invalid element if the stream doesn't exist.
	*/
	VertexElement GetElement(VertexElement::EUsage usage) const
	{
		if(!IsValid())
			return VertexElement::Invalid();

		for(int streamId = 0; streamId < GetStreamCount(); ++streamId) {
			VertexElement e = GetElement(streamId, usage);
			if(e.IsValid())
				return e;
		}

		return VertexElement::Invalid();
	}

	//! Gets a hash for this element
	int GetHash() const
	{
		return m_Hash;
	}

private:
	int CalcHash()
	{
		if(!m_IsValid)
			return 0;

		m_Hash = 7;

		core::HashType<VertexElement> hasher;
		for(int s = 0; s < GetStreamCount(); ++s) {
			for(int e = 0; e < GetElemCount(s); ++s) {
				m_Hash = 31 * m_Hash + hasher(GetElement(s, e));
			}
		}

		return m_Hash;
	}

private:
	core::String m_Name;
	std::shared_ptr<VertexDeclaration> m_Declaration;
	bool m_IsValid;

	int m_Hash;
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