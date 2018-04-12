#ifndef INCLUDED_LUX_VERTEXBUFFER_H
#define INCLUDED_LUX_VERTEXBUFFER_H
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
	Data is copied only if init is null.<br>
	Data is copied element wise.<br>
	Element with same sematic and type are copied.<br>
	New elements are not initialized.<br>
	Missing elements are removed.<br>
	\param format The format to set.
	\param stream Which stream of the format to use
	\param moveOld Should the old data be copied.
	\param init Pointer to the default vertex, null if no default is used.
	*/
	virtual void SetFormat(const VertexFormat& format, int stream, bool moveOld = true, const void* init = nullptr) = 0;

	//! Set the format of the vertexbuffer.
	/**
	Data is copied only if init is null.<br>
	Data is copied element wise.<br>
	Element with same sematic and type are copied.<br>
	New elements are not initialized.<br>
	Missing elements are removed.<br>
	Uses the first stream of the format.
	\param format The format to set.
	\param stream Which stream of the format to use
	\param moveOld Should the old data be copied.
	\param init Pointer to the default vertex, null if no default is used.
	*/
	void SetFormat(const VertexFormat& format, bool moveOld = true, void* init = nullptr)
	{
		SetFormat(format, 0, moveOld, init);
	}

	//! Get the used vertex format
	virtual const VertexFormat& GetFormat() const = 0;
	//! Get the used stream of the vertex format.
	virtual int GetStream() const = 0;

	virtual int AddVertex(const void* vertex) = 0;
	virtual int AddVertices(const void* vertices, int count) = 0;
	virtual void SetVertex(const void* vertex, int n) = 0;
	virtual void SetVertices(const void* vertices, int count, int n) = 0;
	virtual void GetVertex(void* ptr, int n) const = 0;
	virtual void GetVertices(void* ptr, int count, int n) const = 0;

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
	core::ConstStrideRange<T> ConstElements(VertexElement::EUsage element) const
	{
		auto stream = GetStream();
		auto elem = GetFormat().GetElement(stream, element);
		if(!elem.IsValid()) {
			core::ConstStrideIterator<T> p(nullptr, 0);
			return core::ConstStrideRange<T>(p, p);
		}
		auto data = Pointer_c(0, GetSize());
		auto begin = core::ConstStrideIterator<T>(data, GetStride(), elem.offset);
		auto end = begin + GetSize();
		return core::ConstStrideRange<T>(begin, end);
	}
};

} //namespace video
} //namespace lux

#endif