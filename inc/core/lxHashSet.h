#ifndef INCLUDED_LX_HASH_SET_H
#define INCLUDED_LX_HASH_SET_H
#include "core/lxUtil.h"
#include "core/lxIterator.h"
#include "math/lxMath.h"

namespace lux
{
namespace core
{

template <typename T, typename Hash = HashType<T>, typename Compare = CompareType<T>>
class HashSet
{
	/*
	Each bucket contains the index into the entry array.
	The entries with the same hash make a cyclic linked list.
	*/
	struct Entry
	{
		T value;
		size_t next;
	};

	// The value of id that point nowhere.
	// Used to indicate empty buckets.
	static const size_t INVALID_ID = (size_t)-1;

public:
	template <bool IsConst>
	class BaseIterator : public core::BaseIterator<core::BidirectionalIteratorTag, T>
	{
	public:
		using PtrT = typename core::Choose<IsConst, const Entry*, Entry*>::type;
		static const bool IS_CONST = IsConst;
	public:
		BaseIterator() = default;
		explicit BaseIterator(PtrT current) :
			m_Current(current)
		{
		}
		BaseIterator(const BaseIterator<IsConst>& iter) :
			m_Current(iter.m_Current)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseIterator(const BaseIterator<!U>& iter) :
			m_Current(iter.m_Current)
		{
		}

		template <bool U = IsConst, std::enable_if_t<U, int> = 0>
		BaseIterator& operator=(const BaseIterator<!U>& iter)
		{
			m_Current = iter.m_Current;
			return *this;
		}
		BaseIterator& operator=(const BaseIterator<IsConst>& iter)
		{
			m_Current = iter.m_Current;
			return *this;
		}

		BaseIterator& operator++() { ++m_Current; return *this; }
		BaseIterator& operator--() { --m_Current; return *this; }
		BaseIterator operator++(int)
		{
			BaseIterator tmp(*this); ++m_Current; return tmp;
		}
		BaseIterator operator--(int)
		{
			BaseIterator tmp = *this; --m_Current; return tmp;
		}

		template <bool IsConst2>
		bool operator==(const BaseIterator<IsConst2>& other) const
		{
			return m_Current == other.m_Current;
		}
		template <bool IsConst2>
		bool operator!=(const BaseIterator<IsConst2>& other) const
		{
			return m_Current != other.m_Current;
		}

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		T& operator*()
		{
			return m_Current->value;
		}

		const T& operator*() const
		{
			return m_Current->value;
		}

		template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
		T* operator->()
		{
			return &m_Current->value;
		}

		const T* operator->() const
		{
			return &m_Current->value;
		}

	private:
		PtrT m_Current;
	};

	using HashType = Hash;
	using CompareType = Compare;
	using Iterator = BaseIterator<false>;
	using ConstIterator = BaseIterator<true>;

public:
	HashSet() :
		m_Entries(nullptr),
		m_Allocated(0),
		m_Size(0),
		m_Buckets(nullptr),
		m_BucketCount(0),
		m_MaxLoadFactor(1)
	{
	}

	HashSet(size_t allocated, size_t bucketCount = 0) :
		HashSet()
	{
		ReserveAndRehash(allocated, bucketCount);
	}

	HashSet(std::initializer_list<T> values) :
		HashSet(values.begin(), values.end())
	{
	}

	template <typename IterT>
	HashSet(IterT begin, IterT end) :
		HashSet()
	{
		Reserve(core::IteratorDistance(begin, end));
		for(auto it = begin; it != end; ++it)
			Insert(*it);
	}

	HashSet(const HashSet& other) :
		HashSet()
	{
		*this = other;
	}

	HashSet(HashSet&& old) :
		HashSet()
	{
		*this = std::move(old);
	}

	~HashSet()
	{
		Clear();
		LUX_FREE_RAW(m_Entries);
		LUX_FREE_RAW(m_Buckets);
	}

	void Clear()
	{
		for(size_t i = 0; i < m_Size; ++i)
			m_Entries[i].value.~T();
		m_Size = 0;
		for(size_t i = 0; i < m_BucketCount; ++i)
			m_Buckets[i] = INVALID_ID;
	}

	HashSet& operator=(const HashSet& other)
	{
		Clear();

		if(m_Allocated < other.m_Size) {
			LUX_FREE_RAW(m_Entries);
			m_Entries = (Entry*)LUX_NEW_RAW(sizeof(Entry) * other.m_Size);
			m_Allocated = other.m_Size;
		}
		m_Size = other.Size();
		for(size_t i = 0; i < m_Size; ++i)
			new (&m_Entries[i]) Entry(other.m_Entries[i]);

		LUX_FREE_RAW(m_Buckets);
		m_Buckets = (size_t*)LUX_NEW_RAW(sizeof(size_t) * other.m_BucketCount);
		m_BucketCount = other.m_BucketCount;
		memcpy(m_Buckets, other.m_Buckets, sizeof(size_t) * other.m_BucketCount);

		m_MaxLoadFactor = other.m_MaxLoadFactor;

		return *this;
	}

	HashSet& operator=(HashSet&& old)
	{
		this->~HashSet();

		m_Entries = old.m_Entries;
		m_Allocated = old.m_Allocated;
		m_Size = old.m_Size;
		m_Buckets = old.m_Buckets;
		m_BucketCount = old.m_BucketCount;
		m_MaxLoadFactor = old.m_MaxLoadFactor;

		old.m_Entries = nullptr;
		old.m_Allocated = 0;
		old.m_Size = 0;
		old.m_Buckets = nullptr;
		old.m_BucketCount = 0;

		return *this;
	}

	bool operator==(const HashSet& other) const
	{
		if(m_Size != other.Size())
			return false;

		for(auto& v : other) {
			if(!this->HasValue(v))
				return false;
		}
		return true;
	}

	bool operator!=(const HashSet& other) const
	{
		return !(*this == oher);
	}

	bool Insert(const T& value, Iterator* it = nullptr, bool replace = true)
	{
		return BaseInsert<true>((T&&)value, it, replace);
	}

	bool Insert(T&& value, Iterator* it = nullptr, bool replace = true)
	{
		return BaseInsert<false>(std::move(value), it, replace);
	}

	template <typename K>
	bool EraseAny(const K& value)
	{
		if(m_Size == 0)
			return false;
		auto hash = m_Hasher(value) % m_BucketCount;
		size_t prev;
		auto id = GetId<K>(hash, value, &prev);
		if(id == INVALID_ID)
			return false;

		// Remove all references to entry.
		if(m_Entries[id].next == id) {
			m_Buckets[hash] = INVALID_ID;
		} else {
			m_Entries[prev].next = m_Entries[id].next;
			m_Buckets[hash] = prev;
		}

		--m_Size;

		// Move last entry into deleted place, and delete it.
		if(id != m_Size)
			m_Entries[id] = std::move(m_Entries[m_Size]);
		m_Entries[m_Size].value.~T();

		if(id != m_Size) {
			// Update references to moved entry(redirect from m_Size to id)
			auto hash2 = m_Hasher(m_Entries[id].value) % m_BucketCount;
			if(m_Entries[id].next == m_Size) {
				m_Entries[id].next = id;
				m_Buckets[hash2] = id;
			} else {
				size_t prev2 = Previous(m_Size);
				m_Entries[prev2].next = id;
				m_Buckets[hash2] = prev2;
			}
		}

		return true;
	}

	bool Erase(const T& value)
	{
		return EraseAny<T>(value);
	}

	void Erase(const Iterator& it)
	{
		Erase(*it);
	}

	Iterator FindOrAdd(const T& value, bool* added = nullptr)
	{
		Iterator it;
		bool _added = Insert(value, &it, false);
		if(added)
			*added = _added;
		return it;
	}
	Iterator FindOrAdd(T&& old, bool* added = nullptr)
	{
		Iterator it;
		bool _added = Insert(old, &it, false);
		if(added)
			*added = _added;
		return it;
	}

	template <typename K = T>
	Iterator Find(const K& key)
	{
		auto id = GetId<K>(key);
		if(id == INVALID_ID)
			return End();
		return Iterator(&m_Entries[id]);
	}

	template <typename K = T>
	ConstIterator Find(const K& key) const
	{
		auto id = GetId<K>(key);
		if(id == INVALID_ID)
			return End();
		return ConstIterator(&m_Entries[id]);
	}

	template <typename K = T>
	bool HasValueAny(const K& key) const
	{
		auto it = Find(key);
		return (it != End());
	}
	bool HasValue(const T& key) const
	{
		return HasValueAny(key);
	}

	Iterator First() { return Iterator(m_Entries); }
	Iterator End() { return Iterator(m_Entries + m_Size); }
	ConstIterator First() const { return ConstIterator(m_Entries); }
	ConstIterator End() const { return ConstIterator(m_Entries + m_Size); }

	void Reserve(size_t allocated)
	{
		auto bucketCount = (size_t)(allocated / m_MaxLoadFactor);
		ReserveAndRehash(allocated, bucketCount);
	}

	void ReserveAndRehash(size_t allocated, size_t bucketCount)
	{
		bucketCount = GoodBucketCount(bucketCount);

		PureReserve(allocated);

		if(m_BucketCount < bucketCount) {
			auto newBuckets = (size_t*)LUX_NEW_RAW(bucketCount * sizeof(size_t));
			// Reset connections.
			for(size_t i = 0; i < bucketCount; ++i)
				newBuckets[i] = INVALID_ID;

			// Rehash needed.
			for(size_t i = 0; i < m_Size; ++i) {
				auto hash = m_Hasher(m_Entries[i].value) % bucketCount;
				if(newBuckets[hash] == INVALID_ID) {
					newBuckets[hash] = i;
					m_Entries[i].next = i;
				} else {
					// Update cyclic list
					m_Entries[i].next = m_Entries[newBuckets[hash]].next;
					m_Entries[newBuckets[hash]].next = i;
				}
			}
			LUX_FREE_RAW(m_Buckets);
			m_Buckets = newBuckets;
			m_BucketCount = bucketCount;
		}
	}

	float GetLoadFactor() const
	{
		return (float)m_Size / (float)m_BucketCount;
	}
	float GetMaxLoadFactor() const
	{
		return m_MaxLoadFactor;
	}

	size_t Size() const
	{
		return m_Size;
	}
	size_t Allocated() const
	{
		return m_Allocated;
	}
	size_t BucketCount() const
	{
		return m_BucketCount;
	}
	bool IsEmpty() const
	{
		return (Size() == 0);
	}

	const Hash& Hasher() const
	{
		return m_Hasher;
	}
	const Compare& Comparer() const
	{
		return m_Comparer;
	}

private:
	void PureReserve(size_t allocated)
	{
		// No rehash requiered.
		if(m_Allocated >= allocated)
			return;
		Entry* newEntries = (Entry*)LUX_NEW_RAW(allocated * sizeof(Entry));

		// Move construct new values
		for(size_t i = 0; i < m_Size; ++i) {
			new (&newEntries[i]) Entry(std::move(m_Entries[i]));
			m_Entries[i].~Entry();
		}
		LUX_FREE_RAW(m_Entries);
		m_Entries = newEntries;
		m_Allocated = allocated;
	}

	template <typename K>
	size_t GetId(const K& value) const
	{
		if(m_Size == 0)
			return INVALID_ID;
		auto pureHash = m_Hasher(value);
		return GetId<K>(pureHash % m_BucketCount, value);
	}

	template <typename K>
	size_t GetId(size_t hash, const K& value, size_t* _prev = nullptr) const
	{
		if(m_Size == 0)
			return INVALID_ID;
		auto id = m_Buckets[hash];
		if(id == INVALID_ID)
			return INVALID_ID;
		size_t prev = INVALID_ID;
		auto begin = id;
		while(!m_Comparer.Equal(m_Entries[id].value, value)) {
			prev = id;
			id = m_Entries[id].next;
			if(id == begin)
				return INVALID_ID;
		}
		if(_prev) {
			if(prev != INVALID_ID)
				*_prev = prev;
			else
				*_prev = Previous(id);
		}

		return id;
	}

	size_t Previous(size_t i) const
	{
		size_t prev;
		auto cur = i;
		do {
			prev = cur;
			cur = m_Entries[cur].next;
		} while(m_Entries[prev].next != i);
		return prev;
	}

	template <bool Copy>
	bool BaseInsert(T&& value, Iterator* it = nullptr, bool replace = true)
	{
		auto pureHash = m_Hasher(value);
		size_t hash = 0;
		size_t id = 0;
		if(m_Size != 0) {
			hash = pureHash % m_BucketCount;
			id = GetId<T>(hash, value);
			if(id != INVALID_ID) {
				if(replace)
					m_Entries[id].value = (core::Choose<Copy, const T&, T&&>::type)value;
				if(it)
					*it = Iterator(&m_Entries[id]);
				return false;
			}
		}

		size_t oldBucketSize = m_BucketCount;
		if(m_Size == m_Allocated)
			Reserve(NewSize(Size()));
		if(double(m_Size + 1) / m_BucketCount > GetMaxLoadFactor())
			Reserve(NewSize(Size()));

		// If rehash happened => Recalculate hash.
		if(oldBucketSize != m_BucketCount)
			hash = pureHash % m_BucketCount;
		id = m_Buckets[hash];
		if(id == INVALID_ID) {
			m_Buckets[hash] = m_Size;
			m_Entries[m_Size].next = m_Size;
		} else {
			m_Entries[m_Size].next = m_Entries[id].next;
			m_Entries[id].next = m_Size;
		}
		new (&m_Entries[m_Size].value) T(
			(core::Choose<Copy, const T&, T&&>::type)value);
		if(it)
			*it = Iterator(&m_Entries[m_Size]);
		++m_Size;
		return true;
	}

private:
	static size_t NewSize(size_t oldSize)
	{
		if(oldSize == 0)
			return 5;
		return oldSize * 2;
	}
	static size_t NextBucketCount(size_t oldCount)
	{
		return GoodBucketCount((oldCount * 2) / 3);
	}
	static size_t GoodBucketCount(size_t bucketCount)
	{
		if(bucketCount <= 7)
			return 7;
		return math::NextPrime(bucketCount);
	}

private:
	Entry* m_Entries;
	size_t m_Allocated;
	size_t m_Size;

	size_t* m_Buckets;
	size_t m_BucketCount;

	float m_MaxLoadFactor;

	const HashType m_Hasher;
	const CompareType m_Comparer;
};

template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::Iterator begin(HashSet<T, Hash, Compare>& set) { return set.First(); }
template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::Iterator end(HashSet<T, Hash, Compare>& set) { return set.End(); }

template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::ConstIterator begin(const HashSet<T, Hash, Compare>& set) { return set.First(); }
template <typename T, typename Hash, typename Compare>
typename HashSet<T, Hash, Compare>::ConstIterator end(const HashSet<T, Hash, Compare>& set) { return set.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_HASH_SET_H