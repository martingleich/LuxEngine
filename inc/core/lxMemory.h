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
		m_Data(nullptr),
		m_Size(0)
	{
		std::swap(m_Data, old.m_Data);
		std::swap(m_Size, old.m_Size);
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
		std::swap(m_Data, old.m_Data);
		std::swap(m_Size, old.m_Size);
		return *this;
	}

	~RawMemory()
	{
		SetSize(0);
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

		u8* newData = nullptr;
		if(newSize > 0) {
			newData = LUX_NEW_ARRAY(u8, newSize);
			if(action == ZERO) {
				std::memset(newData, 0, newSize);
			} else if(action & COPY) {
				if(m_Size) {
					const size_t toCopy = m_Size < newSize ? m_Size : newSize;
					std::memcpy(newData, m_Data, toCopy);
				}
				if(action & ZERO) {
					std::memset(newData + m_Size, 0, newSize - m_Size);
				}
			} else {
				(void)0;
			}
		}

		LUX_FREE_ARRAY(m_Data);

		m_Data = newData;
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

template <typename T>
class LazyCopy
{
private:
	struct Data
	{
		Data() :
			refCount(1)
		{}

		Data(const T& _value) :
			value(_value),
			refCount(1)
		{}

		T value;
		int refCount;
	};
public:
	LazyCopy()
	{
		m_Ptr = LUX_NEW(Data);
	}
	LazyCopy(const T& value)
	{
		m_Ptr = LUX_NEW(Data)(value);
	}
	LazyCopy(const LazyCopy& other)
	{
		m_Ptr = other.m_Ptr;
		m_Ptr->refCount++;
	}
	LazyCopy(LazyCopy&& old) :
		m_Ptr(nullptr)
	{
		std::swap(m_Ptr, old.m_Ptr);
	}
	~LazyCopy()
	{
		Destroy();
	}
	LazyCopy& operator=(const LazyCopy& other)
	{
		if(this == &other)
			return *this;
		Destroy();
		m_Ptr = other.m_Ptr;
		m_Ptr->refCount++;
		return *this;
	}
	LazyCopy& operator=(LazyCopy&& old)
	{
		std::swap(m_Ptr, old.m_Ptr);
		return *this;
	}

	void ForceCopy()
	{
		if(m_Ptr->refCount > 1) {
			m_Ptr = LUX_NEW(Data)(*m_Ptr);
			m_Ptr->refCount = 1;
		}
	}

	T* operator->() { return &m_Ptr->value; }
	const T* operator->() const { return &m_Ptr->value; }

private:
	void Destroy()
	{
		m_Ptr->refCount--;
		if(m_Ptr->refCount == 0)
			LUX_FREE(m_Ptr);
	}
public:
	Data* m_Ptr;
};

}
}

#endif
