#ifndef INCLUDED_LUX_LX_HASH_SET_H
#define INCLUDED_LUX_LX_HASH_SET_H
#include "core/lxUtil.h"
#include "core/lxIterator.h"
#include "core/BasicHashSet.h"

namespace lux
{
namespace core
{


template <
	typename T,
	typename Hash = HashType<T>,
	typename Compare = CompareType<T>>
	class HashSet
{
public:
	using Iterator = T * ;
	using ConstIterator = const T*;

	struct FindAddResult
	{
		Iterator it;
		bool added;
	};

public:
	HashSet() = default;
	HashSet(const Hash& hasher, const Compare& comparer) :
		m_Base(hasher),
		m_Compare(comparer)
	{
	}
	HashSet(const HashSet& other) = default;
	HashSet(HashSet&& old) = default;
	~HashSet() = default;

	HashSet& operator=(const HashSet& other) = default;
	HashSet& operator=(HashSet&& old) = default;

	void Clear() { m_Base.Clear(); }

	bool Insert(const T& value, bool replace = true)
	{
		return m_Base.Add(value, replace, value).addedNew;
	}

	template <typename T2=T>
	bool Erase(const T& value)
	{
		auto result = m_Base.Find(value);
		return m_Base.Erase(result).removed;
	}

	void EraseIter(Iterator it)
	{
		m_Base.Erase(it - &m_Base.GetValue(0));
	}

	FindAddResult FindOrAdd(const T& value)
	{
		auto result = m_Base.Add(value, false, value);
		return {Iterator(&m_Base.GetValue(result.id)), result.addedNew};
	}

	template <typename T2 = T>
	Iterator Find(const T2& value)
	{
		auto result = m_Base.Find(value);
		if(result.IsValid())
			return Iterator(&m_Base.GetValue(result.id));
		else
			return end();
	}

	template <typename T2 = T>
	ConstIterator Find(const T2& value) const
	{
		auto result = m_Base.Find(value);
		if(result.IsValid())
			return ConstIterator(&m_Base.GetValue(result.id));
		else
			return End();
	}

	template <typename T2 = T>
	bool Exists(const T2& key) const
	{
		return Find(key).IsValid();
	}

	Iterator begin() { return Iterator(&m_Base.GetValue(0)); }
	Iterator end() { return Iterator(&m_Base.GetValue(Size())); }
	ConstIterator begin() const { return ConstIterator(&m_Base.GetValue(0)); }
	ConstIterator end() const { return ConstIterator(&m_Base.GetValue(Size())); }

	void Reserve(int count) { m_Base.Reserve(count); }
	int Size() const { return m_Base.GetSize(); }
	int Allocated() const { return m_Base.GetAllocated(); }
	bool IsEmpty() const { return m_Base.GetSize() == 0; }

private:
	BasicHashSet<T, Hash, Compare> m_Base;
};

template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::Iterator begin(HashSet<T, Hash, Compare>& set) { return set.begin(); }
template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::Iterator end(HashSet<T, Hash, Compare>& set) { return set.end(); }

template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::ConstIterator begin(const HashSet<T, Hash, Compare>& set) { return set.begin(); }
template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::ConstIterator end(const HashSet<T, Hash, Compare>& set) { return set.end(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_LX_HASH_SET_H