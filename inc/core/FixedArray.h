#ifndef INCLUDED_LUX_FIXED_ARRAY_H
#define INCLUDED_LUX_FIXED_ARRAY_H
#include "core/LuxBase.h"
#include <initializer_list>

namespace lux
{
namespace core
{

template <typename T, int SIZE>
class FixedArray
{
	static_assert(SIZE >= 0, "SIZE must be bigger than zero");
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

	Iterator begin() { return m_Data; }
	Iterator end() { return m_Data + SIZE; }

	ConstIterator begin() const { return m_Data; }
	ConstIterator end() const { return m_Data + SIZE; }

	int Size() const
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

	T& operator[](int i)
	{
		lxAssert(i >= 0 && i < SIZE);
		return m_Data[i];
	}
	
	const T& operator[](int i) const
	{
		lxAssert(i >= 0 && i < SIZE);
		return m_Data[i];
	}

private:
	T m_Data[SIZE];
};

template <typename T, int SIZE> typename FixedArray<T, SIZE>::Iterator begin(FixedArray<T, SIZE>& array) { return array.Frist(); }
template <typename T, int SIZE> typename FixedArray<T, SIZE>::Iterator end(FixedArray<T, SIZE>& array) { return array.End(); }
template <typename T, int SIZE> typename FixedArray<T, SIZE>::ConstIterator begin(const FixedArray<T, SIZE>& array) { return array.Frist(); }
template <typename T, int SIZE> typename FixedArray<T, SIZE>::ConstIterator end(const FixedArray<T, SIZE>& array) { return array.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_ARRAY_H