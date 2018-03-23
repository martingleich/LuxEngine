#ifndef INCLUDED_LUX_LX_HASH_MAP_H
#define INCLUDED_LUX_LX_HASH_MAP_H
#include "core/lxHashSet.h"

namespace lux
{
namespace core
{

template <typename K, typename V, typename Hash = HashType<K>, typename Compare = CompareType<K>>
class HashMap
{
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
		Tuple(const Tuple&) = default;
		Tuple(Tuple&& old) :
			key(std::move(old.key)),
			value(std::move(old.value))
		{
		}
		Tuple& operator=(const Tuple&) = default;
		Tuple& operator=(Tuple&& old)
		{
			key = std::move(old.key);
			value = std::move(old.value);
			return *this;
		}
	};
	struct HashTuple
	{
		Hash hasher;
		int operator()(const Tuple& e) const
		{
			return hasher(e.key);
		}
		template <typename KeyT>
		int operator()(const KeyT& key) const
		{
			return hasher(key);
		}
	};
	struct CompareTuple
	{
		Compare comparer;
		bool Equal(const Tuple& a, const Tuple& b) const
		{
			return comparer.Equal(a.key, b.key);
		}
		bool Smaller(const Tuple& a, const Tuple& b) const
		{
			return comparer.Smaller(a.key, b.key);
		}

		template <typename KeyT>
		bool Equal(const Tuple& a, const KeyT& b) const
		{
			return comparer.Equal(a.key, b);
		}
		template <typename KeyT>
		bool Smaller(const Tuple& a, const KeyT& b) const
		{
			return comparer.Smaller(a.key, b);
		}
	};

	using BaseSet = typename HashSet<Tuple, HashTuple, CompareTuple>;

public:
	template <bool IsConst>
	class BaseIterator : public core::BaseIterator<BidirectionalIteratorTag, V>
	{
		friend class HashMap;
	public:
		static const bool IS_CONST = IsConst;

		BaseIterator() = default;
		explicit BaseIterator(typename BaseSet::template BaseIterator<IsConst> it) :
			m_Iterator(it)
		{
		}
		BaseIterator(const BaseIterator<IsConst>& iter) :
			m_Iterator(iter.m_Iterator)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseIterator(const BaseIterator<!U>& iter) :
			m_Iterator(iter.m_Iterator)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseIterator& operator=(const BaseIterator<!U>& iter)
		{
			m_Iterator = iter.m_Iterator;
			return *this;
		}
		BaseIterator& operator=(const BaseIterator<IsConst>& iter)
		{
			m_Iterator = iter.m_Iterator;
			return *this;
		}

		BaseIterator& operator++() { ++m_Iterator; return *this; }
		BaseIterator& operator--() { --m_Iterator; return *this; }
		BaseIterator operator++(int) { BaseIterator tmp(*this); ++m_Iterator; return tmp; }
		BaseIterator operator--(int) { BaseIterator tmp(*this); --m_Iterator; return tmp; }

		template <bool IsConst2>
		bool operator==(const BaseIterator<IsConst2>& other) const
		{
			return m_Iterator == other.m_Iterator;
		}
		template <bool IsConst2>
		bool operator!=(const BaseIterator<IsConst2>& other) const
		{
			return m_Iterator != other.m_Iterator;
		}

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		V& value() { return m_Iterator->value; }
		const V& value() const { return m_Iterator->value; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		K& key() { return m_Iterator->key; }
		const K& key() const { return m_Iterator->key; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		V& operator*() { return m_Iterator->value; }
		const V& operator*() const { return m_Iterator->value; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		V* operator->() { return &m_Iterator->value; }
		const V* operator->() const { return &m_Iterator->value; }

	private:
		typename BaseSet::template BaseIterator<IsConst> m_Iterator;
	};

	template <bool IsConst>
	class BaseKeyIterator : public core::BaseIterator<core::BidirectionalIteratorTag, K>
	{
	public:
		static const bool IS_CONST = IsConst;

		BaseKeyIterator() = default;
		explicit BaseKeyIterator(typename BaseSet:: template BaseIterator<IsConst> it) :
			m_Iterator(it)
		{
		}
		BaseKeyIterator(const BaseKeyIterator<IsConst>& iter) :
			m_Iterator(iter.m_Iterator)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseKeyIterator(const BaseKeyIterator<!U>& iter) :
			m_Iterator(iter.m_Iterator)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseKeyIterator& operator=(const BaseKeyIterator<!U>& iter)
		{
			m_Iterator = iter.m_Iterator;
			return *this;
		}
		BaseKeyIterator& operator=(const BaseKeyIterator<IsConst>& iter)
		{
			m_Iterator = iter.m_Iterator;
			return *this;
		}

		BaseKeyIterator& operator++() { ++m_Iterator; return *this; }
		BaseKeyIterator& operator--() { --m_Iterator; return *this; }
		BaseKeyIterator operator++(int) { BaseKeyIterator tmp(*this); ++m_Iterator; return tmp; }
		BaseKeyIterator operator--(int) { BaseKeyIterator tmp(*this); --m_Iterator; return tmp; }

		template <bool IsConst2>
		bool operator==(const BaseKeyIterator<IsConst2>& other) const
		{
			return m_Iterator == other.m_Iterator;
		}
		template <bool IsConst2>
		bool operator!=(const BaseKeyIterator<IsConst2>& other) const
		{
			return m_Iterator != other.m_Iterator;
		}

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		V& value() { return m_Iterator->value; }
		const V& value() const { return m_Iterator->value; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		K& key() { return m_Iterator->key; }
		const K& key() const { return m_Iterator->key; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		K& operator*() { return m_Iterator->key; }
		const K& operator*() const { return m_Iterator->key; }

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		K* operator->() { return &m_Iterator->key; }
		const K* operator->() const { return &m_Iterator->key; }

	private:
		typename BaseSet::template BaseIterator<IsConst> m_Iterator;
	};

	using KeyT = K;
	using ValueT = V;
	using HashType = Hash;
	using CompareType = Compare;

	using Iterator = BaseIterator<false>;
	using ConstIterator = BaseIterator<true>;
	using KeyIterator = BaseKeyIterator<false>;
	using ConstKeyIterator = BaseKeyIterator<true>;

public:
	HashMap()
	{
	}
	HashMap(int allocated, int bucketCount = 0) :
		m_Set(allocated, bucketCount)
	{
	}

	HashMap(std::initializer_list<K> keys, std::initializer_list<V> values) :
		HashMap(keys.begin(), keys.end(), values.begin(), values.end())
	{
	}

	template <typename IterKeyT, typename IterValueT>
	HashMap(
		IterKeyT keysBegin, IterKeyT keysEnd,
		IterValueT valuesBegin, IterValueT valuesEnd) :
		m_Set(core::IteratorDistance(keysBegin, keysEnd))
	{
		lxAssert(core::IteratorDistance(keysBegin, keysEnd) ==
			core::IteratorDistance(valuesBegin, valuesEnd));
		while(keysBegin != keysEnd) {
			m_Set.Insert(std::move(Tuple(*keysBegin, *valuesBegin)));
			++keysBegin;
			++valuesBegin;
		}
	}

	// Assignment
	HashMap(const HashMap& other) :
		m_Set(other.m_Set)
	{
	}

	HashMap(HashMap&& old) :
		m_Set(std::move(old.m_Set))
	{
	}

	HashMap& operator=(const HashMap& other)
	{
		m_Set = other.m_Set;
		return *this;
	}

	HashMap& operator=(HashMap&& old)
	{
		m_Set = std::move(old.m_Set);
		return *this;
	}

	// Comparision
	// No comparisions functions.

	// Resizing
	void Reserve(int allocated)
	{
		m_Set.Reserve(allocated);
	}

	void ReserveAndRehash(int allocated, int bucketCount)
	{
		m_Set.ReserveAndRehash(allocated, bucketCount);
	}

	void Clear()
	{
		m_Set.Clear();
	}

	// Access 
	//! Set a new key in the map
	/**
	\param key The new key
	\param value The new value
	\param [out] out The iterator to the new element.
	\return True if a new element was created, false if an old one was overwritten.
	*/
	bool Set(const K& key, const V& value, Iterator* out = nullptr)
	{
		typename BaseSet::Iterator it;
		bool added = m_Set.Insert(Tuple(key, value), &it);
		if(out)
			*out = Iterator(it);
		return added;
	}

	//! Set a new key in the map, only if the same key doesnt already exists
	/**
	\param key The new key
	\param value The new value
	\return True if a new element was created, false if an old one already existed.
	*/
	bool SetIfNotExist(const K& key, const V& value)
	{
		auto it = Find(key);
		if(it != End())
			return false;
		return Set(key, value);
	}

	template <typename K2 = K>
	bool HasKey(const K2& key) const
	{
		return Find(key) != End();
	}

	template <typename K2 = K>
	Iterator Find(const K2& key)
	{
		return Iterator(m_Set.Find(key));
	}

	template <typename K2 = K>
	ConstIterator Find(const K2& key) const
	{
		return ConstIterator(m_Set.Find(key));
	}

	bool Erase(const K& key)
	{
		return m_Set.EraseAny(key);
	}

	template <typename K2>
	bool EraseAny(const K2& key)
	{
		return m_Set.EraseAny(key);
	}

	void Erase(Iterator it)
	{
		m_Set.Erase(it.m_Iterator);
	}

	V& operator[](const K& key)
	{
		return At(key);
	}
	const V& operator[](const K& key) const
	{
		return At(key);
	}

	V& At(const K& key, const V& init)
	{
		Tuple tuple(key, init);
		return m_Set.FindOrAdd(std::move(tuple))->value;
	}

	template <typename K2 = K>
	V& At(const K2& key)
	{
		Tuple tuple(key);
		return m_Set.FindOrAdd(std::move(tuple))->value;
	}

	template <typename K2 = K>
	const V& At(const K2& key) const
	{
		return m_Set.Find(key)->value;
	}

	template <typename K2 = K>
	const V& Get(const K2& key) const
	{
		return m_Set.Find(key)->value;
	}

	template <typename K2 = K>
	const V& Get(const K2& key, const V& def) const
	{
		auto it = m_Set.Find(key);
		if(it == m_Set.End())
			return def;
		return it->value;
	}

	// Iterators

	ConstIterator First() const { return ConstIterator(m_Set.First()); }
	ConstIterator End() const { return ConstIterator(m_Set.End()); }

	Iterator First() { return Iterator(m_Set.First()); }
	Iterator End() { return Iterator(m_Set.End()); }

	ConstIterator begin() const { return First(); }
	ConstIterator end() const { return End(); }

	Iterator begin() { return First(); }
	Iterator end() { return End(); }

	ConstKeyIterator FirstKey() const { return ConstKeyIterator(m_Set.First()); }
	ConstKeyIterator EndKey() const { return ConstKeyIterator(m_Set.End()); }

	KeyIterator FirstKey() { return KeyIterator(m_Set.First()); }
	KeyIterator EndKey() { return KeyIterator(m_Set.End()); }

	core::Range<KeyIterator> Keys() { return core::MakeRange(FirstKey(), EndKey()); }
	core::Range<ConstKeyIterator> Keys() const { return core::MakeRange(FirstKey(), EndKey()); }

	core::Range<Iterator> Values() { return core::MakeRange(First(), End()); }
	core::Range<ConstIterator> Values() const { return core::MakeRange(First(), End()); }

	// Infomations
	int Size() const
	{
		return m_Set.Size();
	}
	int Allocated() const
	{
		return m_Set.Allocated();
	}
	bool IsEmpty() const
	{
		return Size() == 0;
	}

	float GetLoadFactor() const
	{
		return m_Set.GetLoadFactor();
	}
	float GetMaxLoadFactor() const
	{
		return m_Set.GetMaxLoadFactor();
	}

	const Hash& Hasher() const
	{
		return m_Set.Hasher().hasher;
	}
	const Compare& Comparer() const
	{
		return m_Set.Comparer().comparer;
	}

private:
	BaseSet m_Set;
};

template <typename K, typename V, typename Hash, typename Compare>
typename HashMap<K, V, Hash, Compare>::Iterator begin(HashMap<K, V, Hash, Compare>& map) { return map.First(); }
template <typename K, typename V, typename Hash, typename Compare>
typename HashMap<K, V, Hash, Compare>::Iterator end(HashMap<K, V, Hash, Compare>& map) { return map.End(); }

template <typename K, typename V, typename Hash, typename Compare>
typename HashMap<K, V, Hash, Compare>::ConstIterator begin(const HashMap<K, V, Hash, Compare>& map) { return map.First(); }
template <typename K, typename V, typename Hash, typename Compare>
typename HashMap<K, V, Hash, Compare>::ConstIterator end(const HashMap<K, V, Hash, Compare>& map) { return map.End(); }

} // namespace core
} // namespace lux

#endif // !INCLUDED_LUX_LX_HASH_MAP_H
