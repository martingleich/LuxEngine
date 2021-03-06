#ifndef INCLUDED_LUX_ORDERED_MAP_H
#define INCLUDED_LUX_ORDERED_MAP_H
#include "core/lxOrderedSet.h"

namespace lux
{
namespace core
{
template <typename K, typename V, typename Compare = core::CompareType<K>>
class OrderedMap
{
private:
	struct Tuple
	{
		K key;
		V value;

		Tuple(const K& k) :
			key(k)
		{
		}
		Tuple(const K& k, const V& v) :
			key(k),
			value(v)
		{
		}
	};

	struct CompareTuple
	{
		bool Equal(const Tuple& a, const Tuple& b) const
		{
			return m_Compare.Equal(a.key, b.key);
		}

		bool Smaller(const Tuple& a, const Tuple& b) const
		{
			return m_Compare.Smaller(a.key, b.key);
		}
		bool Equal(const Tuple& a, const K& b) const
		{
			return m_Compare.Equal(a.key, b);
		}
		bool Smaller(const Tuple& a, const K& b) const
		{
			return m_Compare.Smaller(a.key, key);
		}
		bool Equal(const K& a, const Tuple& b) const
		{
			return m_Compare.Equal(a, b.key);
		}
		bool Smaller(const K& a, const Tuple& b) const
		{
			return m_Compare.Smaller(a, b.key);
		}
		Compare m_Compare;
	};

	using BaseSet = OrderedSet<Tuple, CompareTuple>;
	using BaseIter = typename BaseSet::Iterator;
	using BaseConstIter = typename BaseSet::ConstIterator;

public:
	class Iterator : public core::BaseIterator<core::BidirectionalIteratorTag, V>
	{
		friend class ConstIterator;
	public:
		Iterator(const BaseIter& iter) :
			m_Iter(iter)
		{
		}

		Iterator& operator++()
		{
			++m_Iter;
			return *this;
		}

		Iterator& operator--()
		{
			--m_Iter;
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp = *this;
			++*this;
			return tmp;
		}

		Iterator operator--(int)
		{
			Iterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		V* operator->()
		{
			return &(m_Iter->value);
		}

		V& operator*()
		{
			return m_Iter->value;
		}

		V& value()
		{
			return m_Iter->value;
		}

		const K& key()
		{
			return m_Iter->key;
		}

		const BaseIter& GetBase() const
		{
			return m_Iter;
		}

	private:
		BaseIter m_Iter;
	};

	class ConstIterator : public core::BaseIterator<core::BidirectionalIteratorTag, V>
	{
	public:
		ConstIterator(const BaseConstIter& base) :
			m_Iter(base)
		{
		}

		ConstIterator(const ConstIterator& other) :
			m_Iter(other.m_Iter)
		{
		}

		ConstIterator(const Iterator& other) :
			m_Iter(other.m_Iter)
		{
		}
		ConstIterator& operator++()
		{
			++m_Iter;
			return *this;
		}

		ConstIterator& operator--()
		{
			--m_Iter;

			return *this;
		}

		ConstIterator operator++(int)
		{
			ConstIterator tmp = *this;
			++*this;
			return tmp;
		}

		ConstIterator operator--(int)
		{
			ConstIterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		bool operator==(const ConstIterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const ConstIterator& other) const
		{
			return !(*this == other);
		}

		const V* operator->()
		{
			return &(m_Iter->value);
		}

		const V& operator*()
		{
			return m_Iter->value;
		}

		const V& value()
		{
			return m_Iter->value;
		}

		const K& key()
		{
			return m_Iter->key;
		}

		const BaseConstIter& GetBase() const
		{
			return m_Iter;
		}
	private:
		BaseConstIter m_Iter;
	};

	class KeyIterator : public core::BaseIterator<core::BidirectionalIteratorTag, K>
	{
		friend class ConstKeyIterator;
	public:
		KeyIterator(const BaseIter& iter) :
			m_Iter(iter)
		{
		}

		KeyIterator& operator++()
		{
			++m_Iter;
			return *this;
		}

		KeyIterator& operator--()
		{
			--m_Iter;
			return *this;
		}

		KeyIterator operator++(int)
		{
			KeyIterator tmp = *this;
			++*this;
			return tmp;
		}

		KeyIterator operator--(int)
		{
			KeyIterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const KeyIterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const KeyIterator& other) const
		{
			return !(*this == other);
		}

		K* operator->()
		{
			return &(m_Iter->key);
		}

		K& operator*()
		{
			return m_Iter->key;
		}

		V& value()
		{
			return m_Iter->value;
		}

		const K& key()
		{
			return m_Iter->key;
		}

	private:
		BaseIter m_Iter;
	};

	class ConstKeyIterator : public core::BaseIterator<core::BidirectionalIteratorTag, K>
	{
	public:
		ConstKeyIterator(const BaseConstIter& base) :
			m_Iter(base)
		{
		}

		ConstKeyIterator(const ConstKeyIterator& other) :
			m_Iter(other.m_Iter)
		{
		}

		ConstKeyIterator(const KeyIterator& other) :
			m_Iter(other.m_Iter)
		{
		}
		ConstKeyIterator& operator++()
		{
			++m_Iter;
			return *this;
		}

		ConstKeyIterator& operator--()
		{
			--m_Iter;

			return *this;
		}

		ConstKeyIterator operator++(int)
		{
			ConstKeyIterator tmp = *this;
			++*this;
			return tmp;
		}

		ConstKeyIterator operator--(int)
		{
			ConstKeyIterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const KeyIterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const KeyIterator& other) const
		{
			return !(*this == other);
		}

		bool operator==(const ConstKeyIterator& other) const
		{
			return m_Iter == other.m_Iter;
		}

		bool operator!=(const ConstKeyIterator& other) const
		{
			return !(*this == other);
		}

		const K* operator->()
		{
			return &(m_Iter->key);
		}

		const K& operator*()
		{
			return m_Iter->key;
		}

		const V& value()
		{
			return m_Iter->value;
		}

		const K& key()
		{
			return m_Iter->key;
		}

	private:
		BaseConstIter m_Iter;
	};

public:
	OrderedMap()
	{
	}

	OrderedMap(const OrderedMap& other) :
		m_Set(other.m_Set)
	{
	}

	OrderedMap(OrderedMap&& old) :
		m_Set(std::move(old.m_Set))
	{
	}

	OrderedMap& operator=(const OrderedMap& other)
	{
		m_Set = other.m_Set;
		return *this;
	}

	OrderedMap& operator=(OrderedMap&& other)
	{
		m_Set = std::move(other.m_Set);
		return *this;
	}

	bool operator==(const OrderedMap& other) const
	{
		return m_Set == other.m_Set;
	}

	bool operator!=(const OrderedMap& other) const
	{
		return !(*this == other);
	}

	bool Set(const K& key, const V& value, Iterator* out = nullptr)
	{
		typename BaseSet::Iterator it;
		bool added = m_Set.Insert(Tuple(key, value), &it);
		if(out)
			*out = Iterator(it);
		return added;
	}

	bool SetIfNotExist(const K& key, const V& value)
	{
		auto it = Find(key);
		if(it != End())
			return false;
		return Set(key, value);
	}

	Iterator Find(const K& key)
	{
		return Iterator(m_Set.Find(Tuple(key)));
	}

	ConstIterator Find(const K& key) const
	{
		return ConstIterator(m_Set.Find(Tuple(key)));
	}

	bool Erase(const K& key, Iterator* next = nullptr)
	{
		auto it = Find(key);
		if(it == End()) {
			if(next)
				*next = it;
			return false;
		}
		Iterator n = Erase(it);
		if(next)
			*next = n;
		return true;
	}

	Iterator Erase(const Iterator& it)
	{
		return Iterator(m_Set.Erase(it.GetBase()));
	}

	V& operator[](const K& key)
	{
		return At(key);
	}

	const V& operator[](const K& key) const
	{
		return At(key);
	}

	V& At(const K& key, const V& init = V())
	{
		Tuple tuple(key, init);
		auto it = m_Set.Find(tuple);
		if(it == m_Set.End())
			m_Set.Insert(std::move(tuple), &it);
		return it->value;
	}

	const V& At(const K& key) const
	{
		return m_Set.Find(Tuple(key))->value;
	}

	const V& Get(const K& key, const V& def) const
	{
		auto it = m_Set.Find(Tuple(key));
		if(it == m_Set.End())
			return def;
		else
			return it->value;
	}

	ConstIterator First() const
	{
		return ConstIterator(m_Set.FirstC());
	}

	ConstIterator End() const
	{
		return ConstIterator(m_Set.EndC());
	}

	Iterator First()
	{
		return Iterator(m_Set.First());
	}

	Iterator End()
	{
		return Iterator(m_Set.End());
	}

	ConstKeyIterator FirstKey() const
	{
		return ConstKeyIterator(m_Set.FirstC());
	}

	ConstKeyIterator EndKey() const
	{
		return ConstKeyIterator(m_Set.EndC());
	}

	KeyIterator FirstKey()
	{
		return KeyIterator(m_Set.First());
	}

	KeyIterator EndKey()
	{
		return KeyIterator(m_Set.End());
	}

	Range<Iterator> Values()
	{
		return Range<Iterator>(First(), End());
	}

	Range<ConstIterator> Values() const
	{
		return Range<ConstIterator>(First(), End());
	}
	Range<KeyIterator> Keys()
	{
		return Range<KeyIterator>(FirstKey(), EndKey());
	}

	Range<ConstKeyIterator> Keys() const
	{
		return Range<ConstKeyIterator>(FirstKey(), EndKey());
	}

	int Size() const
	{
		return m_Set.Size();
	}

	bool IsEmpty() const
	{
		return (Size() == 0);
	}

	bool HasKey(const K& key) const
	{
		return (Find(key) != End());
	}

	void Clear()
	{
		m_Set.Clear();
	}

private:
	BaseSet m_Set;
};

template <typename K, typename V, typename Compare>
inline typename OrderedMap<K, V, Compare>::Iterator begin(OrderedMap<K, V, Compare>& map) { return map.First(); }
template <typename K, typename V, typename Compare>
inline typename OrderedMap<K, V, Compare>::Iterator end(OrderedMap<K, V, Compare>& map) { return map.End(); }

template <typename K, typename V, typename Compare>
inline typename OrderedMap<K, V, Compare>::ConstIterator begin(const OrderedMap<K, V, Compare>& map) { return map.First(); }
template <typename K, typename V, typename Compare>
inline typename OrderedMap<K, V, Compare>::ConstIterator end(const OrderedMap<K, V, Compare>& map) { return map.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ORDERED_MAP_H
