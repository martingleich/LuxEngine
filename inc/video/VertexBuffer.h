#ifndef INCLUDED_LUX_VERTEXBUFFER_H
#define INCLUDED_LUX_VERTEXBUFFER_H
#include "video/HardwareBuffer.h"
#include "video/VertexFormat.h"
#include "video/VertexFormat.h"

namespace lux
{
namespace video
{
class VertexFormat;

class VertexBuffer : public HardwareBuffer
{
public:
	LUX_API VertexBuffer(BufferManager* mgr);
	LUX_API ~VertexBuffer();

	//! Set the format of the vertexbuffer.
	/**
	Data is copied only if init is null.<br>
	Data is copied element wise.<br>
	Element with same sematic and type are copied.<br>
	New elements are not initialized.<br>
	Missing elements are removed.<br>
	\param format The format to set.
	\param moveOld Should the old data be copied.
	\param init Pointer to the default vertex, null if no default is used.
	*/
	LUX_API void SetFormat(const VertexFormat& format, bool moveOld = true, const void* init = nullptr);

	//! Get the used vertex format
	const VertexFormat& GetFormat() const
	{
		return m_Format;
	}

	int AddVertex(const void* vertex)
	{
		return AddVertices(vertex, 1);
	}

	int AddVertices(const void* vertices, int count)
	{
		if(m_Cursor + count - 1 >= m_Size) {
			Reserve(m_Cursor + count);
			m_Size += count;
		}

		SetVertices(vertices, count, m_Cursor);

		int ret = m_Cursor;
		m_Cursor += count;
		return ret;
	}

	void SetVertex(const void* vertex, int n)
	{
		SetVertices(vertex, 1, n);
	}
	void SetVertices(const void* vertices, int count, int n)
	{
		std::memcpy(Pointer(n, count), vertices, count*m_Stride);
	}
	void GetVertex(void* ptr, int n) const
	{
		GetVertices(ptr, 1, n);
	}
	void GetVertices(void* ptr, int count, int n) const
	{
		std::memcpy(ptr, Pointer(n, count), count*m_Stride);
	}

	template <typename T>
	core::StrideRange<T> Elements(VertexElement::EUsage element)
	{
		auto elem = GetFormat().GetElement(element);
		if(!elem.IsValid()) {
			core::StrideIterator<T> p(nullptr, 0);
			return core::StrideRange<T>(p, p);
		}
		auto data = Pointer(0, GetSize());
		auto begin = core::StrideIterator<T>(data, GetStride(), elem.GetOffset());
		auto end = begin + GetSize();
		return core::StrideRange<T>(begin, end);
	}

	template <typename T>
	core::ConstStrideRange<T> ConstElements(VertexElement::EUsage element) const
	{
		auto elem = GetFormat().GetElement(element);
		if(!elem.IsValid()) {
			core::ConstStrideIterator<T> p(nullptr, 0);
			return core::ConstStrideRange<T>(p, p);
		}
		auto data = Pointer_c(0, GetSize());
		auto begin = core::ConstStrideIterator<T>(data, GetStride(), elem.GetOffset());
		auto end = begin + GetSize();
		return core::ConstStrideRange<T>(begin, end);
	}

private:
	VertexFormat m_Format;
};

} //namespace video
} //namespace lux

#endif