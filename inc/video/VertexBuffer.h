#ifndef INCLUDED_VERTEXBUFFER_H
#define INCLUDED_VERTEXBUFFER_H
#include "video/HardwareBuffer.h"
#include "video/VertexFormat.h"

namespace lux
{
namespace video
{
class VertexFormat;

class VertexBuffer : public HardwareBuffer
{
public:
	VertexBuffer() : HardwareBuffer(EHardwareBufferType::Vertex) {}

	//! Set the format of the vertexbuffer.
	/**
	All old data is deleted.
	\param format The format to set.
	\param stream Which stream of the format to use
	\param init Pointer to the default vertex, null if no default is used.
	*/
	virtual void SetFormat(const VertexFormat& format, u32 stream, const void* init = nullptr) = 0;

	//! Set the format of the vertexbuffer.
	/**
	All old data is deleted.
	Uses the first stream of the format.
	\param format The format to set.
	\param stream Which stream of the format to use
	\param init Pointer to the default vertex, null if no default is used.
	*/
	void SetFormat(const VertexFormat& format, void* init = nullptr)
	{
		SetFormat(format, 0, init);
	}

	//! Get the used vertex format
	virtual const VertexFormat& GetFormat() const = 0;
	//! Get the used stream of the vertex format.
	virtual u32 GetStream() const = 0;

	virtual u32 AddVertex(const void* vertex) = 0;
	virtual u32 AddVertices(const void* vertices, u32 count) = 0;
	virtual void SetVertex(const void* vertex, u32 n) = 0;
	virtual void SetVertices(const void* vertices, u32 count, u32 n) = 0;
	virtual void GetVertex(void* ptr, u32 n) const = 0;
	virtual void GetVertices(void* ptr, u32 count, u32 n) const = 0;

	template <typename T>
	core::StrideRange<T> Elements(VertexElement::EUsage element)
	{
		auto stream = GetStream();
		auto elem = GetFormat().GetElement(stream, element);
		if(!elem.IsValid()) {
			core::StrideIterator<T> p(nullptr, 0);
			return core::StrideRange<T>(p, p);
		}
		auto data = Pointer(0, GetSize());
		auto begin = core::StrideIterator<T>(data, GetStride(), elem.offset);
		auto end = begin + GetSize();
		return core::StrideRange<T>(begin, end);
	}

	template <typename T>
	core::StrideRange<T> ConstElements(VertexElement::EUsage element) const
	{
		auto stream = GetStream();
		auto elem = GetFormat().GetElement(stream, element);
		if(!elem.IsValid()) {
			core::ConstStrideIterator<T> p(nullptr, 0);
			return core::StrideRange<T>(p, p);
		}
		auto data = Pointer_c(0, GetSize());
		auto begin = core::ConstStrideIterator<T>(data, GetStride(), elem.offset);
		auto end = begin + GetSize();
		return core::StrideRange<T>(begin, end);
	}
};

} //namespace video
} //namespace lux

#endif