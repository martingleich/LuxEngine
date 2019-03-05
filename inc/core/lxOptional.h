#ifndef INCLUDED_LUX_CORE_OPTIONAL_H
#define INCLUDED_LUX_CORE_OPTIONAL_H

namespace lux
{
namespace core
{

template <typename T>
class Optional
{
	// I have to think about possible problems here.
	static_assert(!std::is_reference<T>::value, "Can't use optional reference.");

public:
	using ValueType = T;

	Optional() :
		m_HasValue(false)
	{
	}
	Optional(const T& value) :
		m_HasValue(false)
	{
		SetValue(value);
	}
	Optional(const Optional& other) :
		m_HasValue(false)
	{
		if(other.HasValue())
			SetValue(other.GetValue());
	}
	~Optional()
	{
		Reset();
	}
	
	Optional& operator=(const T& value)
	{
		SetValue(value);
		return *this;
	}
	Optional& operator=(const Optional& other)
	{
		if(other.m_HasValue)
			SetValue(other.GetValue());
		else
			Reset();
		return *this;
	}
	
	bool HasValue() const
	{
		return m_HasValue;
	}
	
	void SetValue(const T& value)
	{
		if(m_HasValue)
			*((T*)m_Data) = value;
		else
			new (m_Data) T(value);
		m_HasValue = true;
	}

	void Reset()
	{
		if(m_HasValue) {
			((T*)m_Data)->~T();
			m_HasValue = false;
		}
	}
	
	T& GetValue() {
		lxAssert(HasValue());
		return *((T*)m_Data);
	}
	const T& GetValue() const
	{
		lxAssert(HasValue());
		return *((T*)m_Data);
	}

private:
	alignas(alignof(T)) char m_Data[sizeof(T)];
	bool m_HasValue;
};

template <typename T>
class Optional<T*>
{
public:
	using ValueType = T*;
	Optional() :
		m_Data(nullptr)
	{
	}
	// May pass nullptr to invalid.
	Optional(ValueType value) :
		m_Data(value)
	{
	}
	Optional(const Optional& other) :
		m_Data(other.m_Data)
	{
	}

	~Optional()
	{
		Reset();
	}
	
	Optional& operator=(ValueType value)
	{
		SetValue(value);
		return *this;
	}
	Optional& operator=(const Optional& other)
	{
		m_Data = other.m_Data;
		return *this;
	}
	
	bool HasValue() const
	{
		return m_Data!=nullptr;
	}
	
	void SetValue(ValueType& value)
	{
		m_Data = value;
	}

	void Reset()
	{
		m_Data = nullptr;
	}
	
	ValueType& GetValue()
	{
		lxAssert(HasValue());
		return m_Data;
	}
	ValueType GetValue() const
	{
		lxAssert(HasValue());
		return m_Data;
	}

private:
	ValueType m_Data;
};

} // namespace core
} // namespace lux
#endif // #ifndef INCLUDED_LUX_CORE_OPTIONAL_H