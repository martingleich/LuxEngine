#ifndef INCLUDED_LX_UNKNOWN_REF_COUNTED_H
#define INCLUDED_LX_UNKNOWN_REF_COUNTED_H

namespace lux
{
template <typename T>
class UnknownRefCounted
{
public:
	UnknownRefCounted() :
		m_Pointer(nullptr)
	{
	}

	UnknownRefCounted(T* ptr) :
		m_Pointer(ptr)
	{
		if(m_Pointer)
			m_Pointer->AddRef();
	}

	void TakeOwnership(T* ptr)
	{
		*this = nullptr;
		m_Pointer = ptr;
	}

	UnknownRefCounted(const UnknownRefCounted& other)
	{
		m_Pointer = other.m_Pointer;
		if(m_Pointer)
			m_Pointer->AddRef();
	}

	~UnknownRefCounted()
	{
		ULONG released = 0;
		if(m_Pointer)
			released = m_Pointer->Release();
	}

	UnknownRefCounted& operator=(std::nullptr_t)
	{
		if(m_Pointer)
			m_Pointer->Release();
		m_Pointer = nullptr;
		return *this;
	}

	UnknownRefCounted& operator=(const UnknownRefCounted& other)
	{
		if(other.m_Pointer)
			other.m_Pointer->AddRef();
		if(m_Pointer)
			m_Pointer->Release();

		m_Pointer = other.m_Pointer;
		return *this;
	}

	template <typename T2 = T>
	operator T2*() const
	{
		return m_Pointer;
	}

	T* operator->() const
	{
		return m_Pointer;
	}

	T** Access()
	{
		*this = nullptr;
		return &m_Pointer;
	}

	operator bool()
	{
		return (m_Pointer != nullptr);
	}

	bool operator!() const
	{
		return (m_Pointer == nullptr);
	}

	bool operator==(UnknownRefCounted other) const
	{
		return m_Pointer == other.m_Pointer;
	}

	bool operator!=(UnknownRefCounted other) const
	{
		return !(*this == other);
	}

private:
	T* m_Pointer;
};

} // namespace lux

#endif // #ifndef INCLUDED_LX_UNKNOWN_REF_COUNTED_H