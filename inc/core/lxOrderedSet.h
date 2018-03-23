#ifndef INCLUDED_LUX_ORDERED_SET_H
#define INCLUDED_LUX_ORDERED_SET_H
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

	bool Insert(T&& value, Iterator* it = nullptr)
	{
		typename Tree::Node* node;
		bool added = m_Tree.Insert(value, &node);
		if(it)
			*it = Iterator(m_Tree.GetRoot(), node);
		return added;
	}

	bool Insert(const T& value, Iterator* it = nullptr)
	{
		typename Tree::Node* node;
		bool added = m_Tree.Insert(value, &node);
		if(it)
			*it = Iterator(m_Tree.GetRoot(), node);
		return added;
	}

	bool Erase(const T& value)
	{
		return m_Tree.Erase(value);
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

	bool HasValue(const T& value)
	{
		auto n = m_Tree.Find(value);

		return (n != nullptr);
	}

	void Clear()
	{
		m_Tree.Clear();
	}

	ConstIterator First() const
	{
		return ConstIterator(m_Tree.GetRoot(), true);
	}

	ConstIterator Last() const
	{
		return ConstIterator(m_Tree.GetRoot(), false);
	}

	ConstIterator Begin() const
	{
		return ConstIterator(m_Tree.GetRoot(), nullptr);
	}

	ConstIterator End() const
	{
		return ConstIterator(m_Tree.GetRoot(), nullptr);
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

	bool IsEmpty() const
	{
		return m_Tree.IsEmpty();
	}

	int Size() const
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
		auto maxErrorCount = Size() - other.Size();
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

template <typename V, typename Compare>
inline typename OrderedSet<V, Compare>::Iterator begin(OrderedSet<V, Compare>& set) { return set.First(); }
template <typename V, typename Compare>
inline typename OrderedSet<V, Compare>::Iterator end(OrderedSet<V, Compare>& set) { return set.End(); }

template <typename V, typename Compare>
inline typename OrderedSet<V, Compare>::ConstIterator begin(const OrderedSet<V, Compare>& set) { return set.First(); }
template <typename V, typename Compare>
inline typename OrderedSet<V, Compare>::ConstIterator end(const OrderedSet<V, Compare>& set) { return set.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ORDERED_SET_H
