#ifndef INCLUDED_FIXED_ARRAY_H
#define INCLUDED_FIXED_ARRAY_H
#include "core/LuxBase.h"
#include <initializer_list>

namespace lux
{
namespace core
{

template <typename T, size_t SIZE>
class FixedArray
{
public:
	using Iterator = T*;
	using ConstIterator = const T*;

	FixedArray()
	{
	}

	FixedArray(std::initializer_list<T> init) :
		m_Data(init)
	{
	}

	Iterator First() { return m_Data; }
	Iterator End() { return m_Data + SIZE; }

	ConstIterator First() const { return m_Data; }
	ConstIterator End() const { return m_Data + SIZE; }

	size_t Size() const
	{
		return SIZE;
	}

	T* Data()
	{
		return m_Data;
	}

	const T* Data() const
	{
		return m_Data;
	}

	T& operator[](size_t i)
	{
		lxAssert(i < SIZE);
		return m_Data[i];
	}
	
	const T& operator[](size_t i) const
	{
		lxAssert(i < SIZE);
		return m_Data[i];
	}

private:
	T m_Data[SIZE];
};

template <typename T, size_t SIZE> typename FixedArray<T, SIZE>::Iterator begin(FixedArray<T, SIZE>& array) { return array.Frist(); }
template <typename T, size_t SIZE> typename FixedArray<T, SIZE>::Iterator end(FixedArray<T, SIZE>& array) { return array.End(); }
template <typename T, size_t SIZE> typename FixedArray<T, SIZE>::ConstIterator begin(const FixedArray<T, SIZE>& array) { return array.Frist(); }
template <typename T, size_t SIZE> typename FixedArray<T, SIZE>::ConstIterator end(const FixedArray<T, SIZE>& array) { return array.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_FIXED_ARRAY_H