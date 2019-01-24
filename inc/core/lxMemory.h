#ifndef INCLUDED_LUX_LXMEMORY_H
#define INCLUDED_LUX_LXMEMORY_H
#include "LuxBase.h"
#include <cstring>
#include <memory>

namespace lux
{
namespace core
{

class RawMemory
{
public:
	static const int NOOP = 0;
	static const int ZERO = 1;
	static const int COPY = 2;

public:
	RawMemory() :
		m_Data(nullptr),
		m_Size(0)
	{
	}

	RawMemory(size_t size, int action = NOOP) :
		m_Data(nullptr),
		m_Size(0)
	{
		SetSize(size, action);
	}

	RawMemory(void* ptr, size_t size) :
		m_Data((u8*)ptr),
		m_Size(size)
	{
	}

	RawMemory(const RawMemory& other, int action = COPY) :
		m_Data(nullptr),
		m_Size(0)
	{
		SetSize(other.m_Size, NOOP);
		if(other.m_Size) {
			if(action == COPY && m_Data)
				std::memcpy(m_Data, other.m_Data, other.m_Size);
			if(action == ZERO && m_Data)
				std::memset(m_Data, 0, other.m_Size);
		}
	}

	RawMemory(RawMemory&& old) :
		m_Data(old.m_Data),
		m_Size(old.m_Size)
	{
		old.m_Size = 0;
		old.m_Data = nullptr;
	}

	void Set(const void* data, int size)
	{
		SetSize(size, NOOP);
		std::memcpy(m_Data, data, size);
	}
	RawMemory& operator=(const RawMemory& other)
	{
		Set(other.m_Data, other.m_Size);
		return *this;
	}

	RawMemory& operator=(RawMemory&& old)
	{
		LUX_FREE_ARRAY(m_Data);

		m_Data = old.m_Data;
		m_Size = old.m_Size;
		old.m_Data = nullptr;
		old.m_Size = 0;

		return *this;
	}

	~RawMemory()
	{
		LUX_FREE_ARRAY(m_Data);
	}

	void SetSize(size_t newSize, int action = NOOP)
	{
		if(m_Size == newSize) {
			if(action & COPY) {
				(void)0;
			} else if(action == NOOP) {
				(void)0;
			} else if(action == ZERO) {
				std::memset(m_Data, 0, m_Size);
			}

			return;
		}

		if(newSize == 0) {
			LUX_FREE_ARRAY(m_Data);
			return;
		}

		u8* data = LUX_NEW_ARRAY(u8, newSize);
		if(action == ZERO) {
			std::memset(data, 0, newSize);
		} else if(action & COPY) {
			if(m_Size) {
				const size_t toCopy = m_Size < newSize ? m_Size : newSize;
				std::memcpy(data, m_Data, toCopy);
			}
			if(action & ZERO) {
				std::memset(data + m_Size, 0, newSize - m_Size);
			}
		} else {
			(void)0;
		}

		if(m_Data)
			LUX_FREE_ARRAY(m_Data);

		m_Data = data;
		m_Size = newSize;
	}

	void SetMinSize(size_t newSize, int action = NOOP)
	{
		if(m_Size >= newSize)
			return;

		SetSize(newSize, action);
	}

	void* Pointer() { return m_Data; }
	const void* Pointer() const { return m_Data; }
	const void* PointerC() const { return Pointer(); }
	template <typename T>
	operator T*()
	{
		return (T*)Pointer();
	}

	template <typename T>
	operator const T*() const
	{
		return (const T*)Pointer();
	}

	template <typename T = void>
	T* GetChangable() const
	{
		return (T*)m_Data;
	}

	size_t GetSize() const
	{
		return m_Size;
	}

private:
	u8* m_Data;
	size_t m_Size;
};

// TODO: Memory without size
// TODO: Memory with optional delete on drop.
}
}

#endif
