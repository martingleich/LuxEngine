#ifndef INCLUDED_LX_UNKNOWN_REF_COUNTED_H
#define INCLUDED_LX_UNKNOWN_REF_COUNTED_H

#ifdef LUX_COMPILE_WITH_D3D9
#include "StrippedD3D9.h"

namespace lux
{
template <typename T>
class UnknownRefCounted
{
public:
	UnknownRefCounted() :
		m_Pointer(nullptr)
	{}

	UnknownRefCounted(T* ptr) :
		m_Pointer(ptr)
	{
	}

	UnknownRefCounted(const UnknownRefCounted& other)
	{
		m_Pointer = other.m_Pointer;
		if(m_Pointer)
			m_Pointer->AddRef();
	}

	~UnknownRefCounted()
	{
		if(m_Pointer)
			m_Pointer->Release();
	}

	UnknownRefCounted& operator=(T* ptr)
	{
		if(m_Pointer)
			m_Pointer->Release();
		m_Pointer = ptr;

		return *this;
	}

	UnknownRefCounted& operator=(const UnknownRefCounted& other)
	{
		if(m_Pointer)
			m_Pointer->Release();
		m_Pointer = other.m_Pointer;
		if(m_Pointer)
			m_Pointer->AddRef();

		return *this;
	}

	operator T*() const
	{
		return m_Pointer;
	}

	T* operator->() const
	{
		return m_Pointer;
	}

private:
	T* m_Pointer;
};

}

#endif

#endif // #ifndef INCLUDED_LX_UNKNOWN_REF_COUNTED_H