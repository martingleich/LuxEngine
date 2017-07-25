#ifndef INCLUDED_LX_ORDERED_SET_H
#define INCLUDED_LX_ORDERED_SET_H
#include "core/lxRedBlack.h"

namespace lux
{
namespace core
{

template <typename T, typename Compare = core::CompareType<T>>
class OrderedSet
{
private:
	using Tree = RedBlackTree<T, Compare>;

public:
	using CompareT = Compare;
	using ValueT = T;
	using ConstIterator = typename Tree::ConstIterator;
	using Iterator = typename Tree::Iterator;

public:
	OrderedSet()
	{
	}

	template <typename ItT>
	OrderedSet(ItT sortedFirst, ItT sortedEnd)
	{
		Tree::FromOrdered(sortedFirst, sortedEnd, &m_Tree);
	}

	OrderedSet(const OrderedSet& other) :
		m_Tree(other.m_Tree)
	{
	}

	OrderedSet(OrderedSet&& old) :
		m_Tree(std::move(old.m_Tree))
	{
	}

	OrderedSet& operator=(const OrderedSet& other)
	{
		m_Tree = other.m_Tree;
		return *this;
	}

	OrderedSet& operator=(OrderedSet&& old)
	{
		m_Tree = std::move(old.m_Tree);
		return *this;
	}

	bool operator==(const OrderedSet& other) const
	{
		return m_Tree == other.m_Tree;
	}

	bool operator!=(const OrderedSet& other) const
	{
		return !(*this == other);
	}

	Iterator Insert(T&& value)
	{
		auto n = m_Tree.Insert(value);

		return Iterator(m_Tree.GetRoot(), n);
	}

	Iterator Insert(const T& value)
	{
		auto n = m_Tree.Insert(value);

		return Iterator(m_Tree.GetRoot(), n);
	}

	void Erase(const T& value)
	{
		m_Tree.Erase(value);
	}

	Iterator Erase(const Iterator& it)
	{
		auto n = m_Tree.Erase(it.GetNode());

		return Iterator(m_Tree.GetRoot(), n);
	}

	Iterator Find(const T& value)
	{
		auto n = m_Tree.Find(value);

		return (n ? Iterator(m_Tree.GetRoot(), n) : End());
	}

	ConstIterator Find(const T& value) const
	{
		auto n = m_Tree.Find(value);

		return (n ? ConstIterator(m_Tree.GetRoot(), n) : End());
	}

	bool HasKey(const T& value)
	{
		auto n = m_Tree.Find(value);

		return (n != nullptr);
	}

	void Clear()
	{
		m_Tree.Clear();
	}

	ConstIterator FirstC() const
	{
		return ConstIterator(m_Tree.GetRoot(), true);
	}

	ConstIterator LastC() const
	{
		return ConstIterator(m_Tree.GetRoot(), false);
	}

	ConstIterator BeginC() const
	{
		return ConstIterator(m_Tree.GetRoot(), nullptr);
	}

	ConstIterator EndC() const
	{
		return ConstIterator(m_Tree.GetRoot(), nullptr);
	}

	ConstIterator First() const
	{
		return FirstC();
	}

	ConstIterator Last() const
	{
		return LastC();
	}

	ConstIterator Begin() const
	{
		return BeginC();
	}

	ConstIterator End() const
	{
		return EndC();
	}

	Iterator First()
	{
		return Iterator(m_Tree.GetRoot(), true);
	}

	Iterator Last()
	{
		return Iterator(m_Tree.GetRoot(), false);
	}

	Iterator Begin()
	{
		return Iterator(m_Tree.GetRoot(), nullptr);
	}

	Iterator End()
	{
		return Iterator(m_Tree.GetRoot(), nullptr);
	}

	//! Support for foreach loop
	Iterator begin()
	{
		return First();
	}

	//! Support for foreach loop
	Iterator end()
	{
		return End();
	}

	//! Support for foreach loop
	ConstIterator begin() const
	{
		return FirstC();
	}

	//! Support for foreach loop
	ConstIterator end() const
	{
		return EndC();
	}
	bool IsEmpty() const
	{
		return m_Tree.IsEmpty();
	}

	size_t Size() const
	{
		return m_Tree.Size();
	}

	const T& Max() const
	{
		return *Last();
	}

	const T& Min() const
	{
		return *First();
	}

	//! Test if other is a subset of this set
	bool IsSubset(const OrderedSet& other) const
	{
		if(other.Size() > Size())
			return false;
		if(other.Size() == 0)
			return true;

		auto jt = other.First();
		auto& comp = m_Tree.GetCompare();
		size_t maxErrorCount = Size() - other.Size();
		for(auto it = First(); it != End(); ++it) {
			if(comp.Equal(*it, *jt)) {
				++jt; // Found this element, next one
			} else {
				// Made an error, check if we can make any more
				if(maxErrorCount == 0)
					return false;
				else
					--maxErrorCount;
			}
			
			if(jt == other.End())
				return true; // Found each element in the subset
		}

		return false;
	}

	//! Test if other is a proper subset of this set
	bool IsProperSubset(const OrderedSet& other) const
	{
		if(Size() == other.Size())
			return false;
		else
			return IsSubset(other);
	}

private:
	Tree m_Tree;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_ORDERED_SET_H