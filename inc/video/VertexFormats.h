#ifndef INCLUDED_VERTEX_FORMATS_H
#define INCLUDED_VERTEX_FORMATS_H
#include "core/lxArray.h"
#include "core/lxString.h"
#include "math/lxMath.h"

namespace lux
{
namespace video
{

// Ein element einer Vertexdeklaration
struct VertexElement
{
	enum class EType
	{
		Float1, // 1 Float liegt als (Wert, 0, 0, 1) im Register
		Float2, // 2 Floats liegen als (Wert, Wert, 0, 1) im Register
		Float3, // 3 Floats liegen als (Wert, Wert, Wert, 1) im Register
		Float4, // 4 Floats
		Color,  // DWORD-Farbe
		Byte4,  // 4 einzelne bytes
		Short2, // 2 WORDS liegen als (Wert, Wert, 0, 1) im Register
		Short4, // 4 WORDS
		Unknown,
	};

	enum class EUsage
	{
		Position = 0,        // D3D/GL: POSITION
		PositionNT,        // D3D/GL: POSITIONNT
		Normal,            // D3D/GL: NORMAL        
		Tangent,            // D3D/GL: TANGENT        
		Binormal,            // D3D/GL: BINORMAL    
		Texcoord0,            // D3D/GL: TEXCOORD0     
		Texcoord1,            // D3D/GL: TEXCOORD1    
		Texcoord2,            // D3D/GL: TEXCOORD2    
		Texcoord3,            // D3D/GL: TEXCOORD3    
		Diffuse,            // D3D: COLOR0         GL: DIFFUSE
		Specular,            // D3D: COLOR1         GL: SPECULAR
		BlendWeight,        // D3D/GL: BLENDWEIGHT    
		BlendIndices,        // D3D/GL: BLENDINDICES 
		Sample,            // D3D/GL: SAMPLE
		Unknown
	};

	u8 stream;         // Der Datenstrom für dieses element
	u16 offset;        // Der Abstand zum Beginn der Defintion pro Datenstrom
	EType type;        // Der Typ der Daten
	EUsage sematic;    // Das Sematic für dieses element

	static VertexElement Invalid()
	{
		return VertexElement();
	}

	VertexElement() : stream(0xFF), offset(0), type(EType::Unknown), sematic(EUsage::Unknown)
	{
	}
	VertexElement(u8 _stream, u16 _offset, EType _type, EUsage _sematic)
		: stream(_stream), offset(_offset), type(_type), sematic(_sematic)
	{
	}

	bool IsValid() const
	{
		return (type != EType::Unknown && sematic != EUsage::Unknown);
	}

	bool operator==(const VertexElement& other) const
	{
		return stream == other.stream && offset == other.offset && type == other.type && sematic == other.sematic;
	}

	bool operator!=(const VertexElement& other) const
	{
		return !(*this == other);
	}
};

} 


namespace core
{
template <>
struct HashType<video::VertexElement>
{
	size_t operator()(const video::VertexElement& e) const
	{
		if(!e.IsValid())
			return 0;

		size_t out = 7;
		out += 31 * out + e.stream;
		out += 31 * out + e.offset;
		out += 31 * out + (size_t)e.sematic;
		out += 31 * out + (size_t)e.type;
		return out;
	}
};

} 


namespace video
{

//! The declaration of a vertex type
/**
A vertex type contains multiple stream containing multiple elements.
A newly created declaration contains only the default stream.
*/
class VertexDeclaration
{
public:
	VertexDeclaration() :
		m_CurrentStream(nullptr),
		m_Stride(0),
		m_IsNormalized(true)
	{
		AddStream();
	}

	void Clear()
	{
		m_Data.Clear();
		m_Streams.Clear();

		m_CurrentStream = nullptr;
		m_Stride = 0;

		m_IsNormalized = true;
		AddStream();
	}

	void AddStream()
	{
		u32 firstElem = m_Data.IsEmpty() ? 0 : ((u32)m_Data.Size() - 1);
		m_Streams.Push_Back(Stream(firstElem));

		m_CurrentStream = &m_Streams[m_Streams.Size() - 1];
		m_IsNormalized = false;
	}

	void AddElement(VertexElement::EUsage usage, VertexElement::EType type = VertexElement::EType::Unknown)
	{
		if(type == VertexElement::EType::Unknown)
			type = GetSematicDefaultType(usage);

		u32 offset;
		if(m_CurrentStream->elemCount == 0)
			offset = 0;
		else
			offset = m_CurrentStream->stride;

		m_Data.Push_Back(Element(offset, usage, type));
		m_CurrentStream->stride = math::Max(m_CurrentStream->stride, offset) + GetTypeSize(type);
		m_CurrentStream->elemCount += 1;

		m_Stride += GetTypeSize(type);
		m_IsNormalized = false;
	}

	void AddElement(u32 offset, VertexElement::EUsage usage, VertexElement::EType type = VertexElement::EType::Unknown)
	{
		if(type == VertexElement::EType::Unknown)
			type = GetSematicDefaultType(usage);

		m_Data.Push_Back(Element(offset, usage, type));
		m_CurrentStream->stride = math::Max(m_CurrentStream->stride, offset) + GetTypeSize(type);
		m_CurrentStream->elemCount += 1;

		m_Stride += GetTypeSize(type);

		m_IsNormalized = false;
	}

	u32 GetElemCount() const
	{
		return (u32)m_Data.Size();
	}

	u32 GetElemCount(u32 stream) const
	{
		if(stream < m_Streams.Size())
			return m_Streams[stream].elemCount;
		else
			return 0;
	}

	u32 GetStreamCount() const
	{
		return (u32)m_Streams.Size();
	}

	VertexElement GetElement(u32 stream, u32 elem) const
	{
		if(stream >= m_Streams.Size())
			return VertexElement::Invalid();

		u32 firstElem = m_Streams[stream].firstElement;
		if(firstElem >= m_Streams.Size())
			return VertexElement::Invalid();

		if(elem >= m_Streams[stream].elemCount)
			return VertexElement::Invalid();

		Element e = m_Data[firstElem + elem];
		return VertexElement((u8)stream, (u16)e.offset, e.type, e.usage);
	}

	u32 GetStride() const
	{
		return m_Stride;
	}

	u32 GetStride(u32 stream) const
	{
		if(stream < m_Streams.Size())
			return m_Streams[stream].stride;
		else
			return 0;
	}

	bool IsValid() const
	{
		if(m_Streams.Size() == 0)
			return false;

		for(u32 streamID = 0; streamID < m_Streams.Size(); ++streamID) {
			const Stream& stream = m_Streams[streamID];
			if(stream.elemCount == 0)
				return false;
			for(u32 elemID = 0; elemID < stream.elemCount; ++elemID) {
				const Element& elem = m_Data[stream.firstElement + elemID];
				if(elem.type == VertexElement::EType::Unknown)
					return false;
				if(elem.usage == VertexElement::EUsage::Unknown)
					return false;
			}
		}

		return true;
	}

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

		for(size_t streamId = m_Streams.Size(); streamId < m_Streams.Size(); ++streamId) {
			const auto& stream = m_Streams[streamId];

			core::Heapsort(m_Data.Data() + stream.firstElement,
				stream.elemCount,
				SortByOffset());
		}

		m_IsNormalized = true;
	}

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
		}

		return VertexElement::EType::Unknown;
	}

private:
	struct Element
	{
	public:
		Element() :
			offset(0),
			usage(VertexElement::EUsage::Unknown),
			type(VertexElement::EType::Unknown)
		{}

		Element(u32 _offset, VertexElement::EUsage _usage, VertexElement::EType _type) :
			offset(_offset),
			usage(_usage),
			type(_type)
		{
		}

		u32 offset;
		VertexElement::EUsage usage;
		VertexElement::EType type;
	};

	struct Stream
	{
		Stream() :
			elemCount(0),
			stride(0),
			firstElement(0)
		{}

		Stream(u32 firstElem) :
			elemCount(0),
			stride(0),
			firstElement(firstElem)
		{
		}

		u32 elemCount;
		u32 stride;

		u32 firstElement;
	};

private:
	core::array<Stream> m_Streams;
	core::array<Element> m_Data;

	Stream* m_CurrentStream;
	u32 m_Stride;

	bool m_IsNormalized;
};

class VertexFormat
{
public:
	LUX_API static const VertexFormat STANDARD;
	LUX_API static const VertexFormat TRANSFORMED;
	LUX_API static const VertexFormat TWO_TEXTURE;
	LUX_API static const VertexFormat TANGENTS;
	LUX_API static const VertexFormat TEXTURE_3D;
	LUX_API static const VertexFormat STANDARD_2D;

public:
	VertexFormat() :
		m_Declaration(nullptr),
		m_IsValid(false),
		m_Hash(0)
	{
	}

	VertexFormat(const string& name, const VertexDeclaration& decl) :
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

		assert(m_Declaration->IsNormalized());
		if(GetStreamCount() != other.GetStreamCount())
			return false;
		if(GetElemCount() != other.GetElemCount())
			return false;

		for(u32 streamId = 0; streamId < GetStreamCount(); ++streamId) {
			for(u32 elemId = 0; elemId < GetElemCount(streamId); ++elemId) {
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

	const string& GetName() const
	{
		return m_Name;
	}

	u32 GetElemCount() const
	{
		if(IsValid())
			return m_Declaration->GetElemCount();
		else
			return 0;
	}

	u32 GetElemCount(u32 stream) const
	{
		if(IsValid())
			return m_Declaration->GetElemCount(stream);
		else
			return 0;
	}

	u32 GetStreamCount() const
	{
		if(IsValid())
			return m_Declaration->GetStreamCount();
		else
			return 0;
	}

	VertexElement GetElement(u32 stream, u32 elem) const
	{
		if(IsValid())
			return m_Declaration->GetElement(stream, elem);
		else
			return VertexElement::Invalid();
	}

	u32 GetStride(u32 stream) const
	{
		if(IsValid())
			return m_Declaration->GetStride(stream);
		else
			return 0;
	}

	u32 GetStride() const
	{
		if(IsValid())
			return m_Declaration->GetStride();
		else
			return 0;
	}

	bool IsValid() const
	{
		return m_Declaration != nullptr && m_IsValid;
	}

	VertexElement GetElement(u32 stream, VertexElement::EUsage usage) const
	{
		if(!IsValid() || stream >= GetStreamCount())
			return VertexElement::Invalid();

		for(u32 elemId = 0; elemId < GetElemCount(stream); ++elemId) {
			VertexElement e = GetElement(stream, elemId);
			if(e.IsValid() && e.sematic == usage)
				return e;
		}

		return VertexElement::Invalid();
	}

	VertexElement GetElement(VertexElement::EUsage usage) const
	{
		if(!IsValid())
			return VertexElement::Invalid();

		for(u32 streamId = 0; streamId < GetStreamCount(); ++streamId) {
			VertexElement e = GetElement(streamId, usage);
			if(e.IsValid())
				return e;
		}

		return VertexElement::Invalid();
	}

	size_t GetHash() const
	{
		return m_Hash;
	}

private:
	size_t CalcHash()
	{
		if(!m_IsValid)
			return 0;

		m_Hash = 7;

		core::HashType<VertexElement> hasher;
		for(u32 s = 0; s < GetStreamCount(); ++s) {
			for(u32 e = 0; e < GetElemCount(s); ++s) {
				m_Hash = 31 * m_Hash + hasher(GetElement(s, e));
			}
		}

		return m_Hash;
	}

private:
	string m_Name;
	std::shared_ptr<VertexDeclaration> m_Declaration;
	bool m_IsValid;

	size_t m_Hash;
};

}    


namespace core
{
template <>
struct HashType<video::VertexFormat>
{
	size_t operator()(const video::VertexFormat& e) const
	{
		return e.GetHash();
	}
};

}
}    


#endif // #ifndef INCLUDED_VERTEX_FORMATS_H