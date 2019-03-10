#ifndef INCLUDED_LX_ANY_ITERATOR_H
#define INCLUDED_LX_ANY_ITERATOR_H
#include "core/iterators/lxRanges.h"

namespace lux
{
namespace core
{

template <typename T>
class VirtualConstIterator
{
public:
	virtual ~VirtualConstIterator() {}
	virtual void Next() = 0;
	virtual void Assign(const VirtualConstIterator* other) = 0;
	virtual bool Equal(const VirtualConstIterator* other) const = 0;
	virtual const T* GetPtr() const = 0;
	virtual VirtualConstIterator* Clone() const = 0;
};

template <typename BaseIterT>
class ConstIteratorImplementation : public VirtualConstIterator<typename BaseIterT::ValueType>
{
public:
	using ValueType = typename BaseIterT::ValueType;
	explicit ConstIteratorImplementation(BaseIterT base) :
		m_BaseIter(base)
	{
	}
	void Next() { ++m_BaseIter; }
	void Assign(const VirtualConstIterator* other) { m_BaseIter = dynamic_cast<const ConstIteratorImplementation&>(*other).m_BaseIter; }
	bool Equal(const VirtualConstIterator* other) const { return Equal(dynamic_cast<const ConstIteratorImplementation*>(other)); }
	bool Equal(const ConstIteratorImplementation* other) const { return other && m_BaseIter == other->m_BaseIter; }
	const ValueType* GetPtr() const { return m_BaseIter.operator->(); }
	VirtualConstIterator* Clone() const { return new ConstIteratorImplementation(m_BaseIter); }

private:
	BaseIterT m_BaseIter;
};

template <typename T>
class AnyBaseConstIterator : public core::BaseIterator<core::ForwardIteratorTag, T>
{
	using AbstractType = VirtualConstIterator<T>;
public:
	AnyBaseConstIterator() :
		m_Iter(nullptr)
	{
	}
	AnyBaseConstIterator(const AnyBaseConstIterator& other) :
		m_Iter(other.m_Iter ? other.m_Iter->Clone() : nullptr)
	{
	}
	AnyBaseConstIterator(AnyBaseConstIterator&& old) :
		m_Iter(nullptr)
	{
		std::swap(m_Iter, old.m_Iter);
	}
	explicit AnyBaseConstIterator(AbstractType* ptr) :
		m_Iter(ptr)
	{
	}
	~AnyBaseConstIterator()
	{
		delete m_Iter;
	}
	AnyBaseConstIterator& operator=(const AnyBaseConstIterator& other)
	{
		if(m_Iter)
			m_Iter->Assign(other.m_Iter);
		else
			m_Iter = other.m_Iter->Clone();
		return *this;
	}
	AnyBaseConstIterator& operator++()
	{
		m_Iter->Next();
		return *this;
	}
	AnyBaseConstIterator operator++(int)
	{
		auto tmp = *this;
		m_Iter->Next();
		return tmp;
	}

	bool operator==(const AnyBaseConstIterator& other) const { return m_Iter->Equal(other.m_Iter); }
	bool operator!=(const AnyBaseConstIterator& other) const { return !m_Iter->Equal(other.m_Iter); }

	const T& operator*() const { return *(m_Iter->GetPtr()); }
	const T* operator->() const { return m_Iter->GetPtr(); }

private:
	AbstractType* m_Iter;
};

template <typename IterT>
inline AnyBaseConstIterator<typename IterT::ValueType> MakeAnyConstIter(IterT it)
{
	return AnyBaseConstIterator<typename IterT::ValueType>(new ConstIteratorImplementation<IterT>(it));
}

template <typename T>
using AnyRange = Range<AnyBaseConstIterator<T>>;

template <typename IterT>
inline AnyRange<typename IterT::ValueType> MakeAnyRange(IterT first, IterT end)
{
	return AnyRange<typename IterT::ValueType>(MakeAnyConstIter(first), MakeAnyConstIter(end));
}

template <typename RangeT>
auto MakeAnyRange(RangeT&& range)
{
	using namespace std;
	using IterT = decltype(begin(range));
	return MakeAnyRange<IterT>(begin(range), end(range));
}

template <typename T>
inline AnyRange<T> MakeEmptyAnyRange()
{
	return AnyRange<T>(MakeAnyConstIter<T*>(nullptr), MakeAnyConstIter<T*>(nullptr));
}

} // namespace core
} // namespace lux
#endif // #ifndef INCLUDED_LX_ANY_ITERATOR_H

