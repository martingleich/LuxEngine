#ifndef INCLUDED_LX_BASIC_HASH_SET_H
#define INCLUDED_LX_BASIC_HASH_SET_H

namespace lux
{
namespace core
{

template <typename T, typename HasherT, typename ComparerT>
class BasicHashSet
{
	struct IdResult
	{
		int id;
		int prevId;
	};

	static const int INVALID_ID = -1;
	using HashT = unsigned int;

public:
	enum class EAddOption
	{
		Replace,
		FailOnDuplicate,
	};
	struct AddResult
	{
		int id;
		bool addedNew;
	};
	struct FindResult
	{
		int id;
		int bucketId;
		int prevId;
		bool IsValid() const { return id != INVALID_ID; }
	};
	struct EraseResult
	{
		bool removed;
	};
	
	BasicHashSet() {}
	BasicHashSet(const HasherT& hasher, const ComparerT& comparer) :
		m_Hasher(hasher),
		m_Comparer(comparer)
	{
	}
	BasicHashSet(const BasicHashSet& other)
	{
		*this = other;
	}
	BasicHashSet(BasicHashSet&& old)
	{
		*this = std::move(old);
	}
	~BasicHashSet()
	{
		Clear();
		Free(m_Values);
		m_Values = nullptr;
		m_Allocated = 0;
		Free(m_NextIds);
		m_NextIds = nullptr;
		Free(m_Buckets);
		m_Buckets = nullptr;
		m_BucketCount = 0;
	}

	BasicHashSet& operator=(const BasicHashSet& other)
	{
		if(this == &other)
			return *this;

		Clear();
		if(m_Allocated < other.m_Size) {
			Free(m_Values);
			m_Values = (T*)Alloc(sizeof(T)*other.m_Size);
			Free(m_NextIds);
			m_NextIds = (int*)Alloc(sizeof(int)*other.m_Size);
			m_Allocated = other.m_Size;
		}
		if(m_BucketCount < other.m_BucketCount) {
			Free(m_Buckets);
			m_Buckets = (int*)Alloc(sizeof(int)*other.m_BucketCount);
		}

		for(int i = 0; i < other.m_Size; ++i)
			new (m_Values + i) T(other.m_Values[i]);
		for(int i = 0; i < other.m_Size; ++i)
			m_NextIds[i] = other.m_NextIds[i];
		for(int i = 0; i < other.m_BucketCount; ++i)
			m_Buckets[i] = other.m_Buckets[i];

		m_Size = other.m_Size;
		m_BucketCount = other.m_BucketCount;

		m_Hasher = other.m_Hasher;
		m_Comparer = other.m_Comparer;
		return *this;
	}
	BasicHashSet& operator=(BasicHashSet&& old)
	{
		if(this == &old)
			return *this;
		std::swap(m_Values, old.m_Values);
		std::swap(m_NextIds, old.m_NextIds);

		std::swap(m_Allocated, old.m_Allocated);
		std::swap(m_Size, old.m_Size);

		std::swap(m_Buckets, old.m_Buckets);
		std::swap(m_BucketCount, old.m_BucketCount);

		std::swap(m_Hasher, old.m_Hasher);
		std::swap(m_Comparer, old.m_Comparer);
		return *this;
	}

	void Clear()
	{
		for(int i = 0; i < m_Size; ++i)
			m_Values[i].~T();
		m_Size = 0;
		for(int i = 0; i < m_BucketCount; ++i)
			m_Buckets[i] = INVALID_ID;
	}

	template <typename T2, typename T3>
	AddResult Add(const T2& keyValue, EAddOption option, const T3& setValue)
	{
		HashT baseHash = GetBaseHash(keyValue);
		if(m_Size != 0) {
			int hash = GetHash(baseHash);
			auto result = GetId(hash, keyValue);
			if(result.id != INVALID_ID) {
				if(option == EAddOption::Replace)
					m_Values[result.id] = setValue;
				return {result.id, false};
			}
		}

		if(m_Size == m_Allocated || double(m_Size + 1) > 1 * m_BucketCount) // 1 is MAX_LOAD_FACTOR
			ReserveAndRehash(m_Size + 1);

		// Place value in list
		int id = m_Size;
		m_Size += 1;
		new (m_Values + id) T(setValue);

		int hash = GetHash(baseHash);
		AddToBucketList(hash, id);
		return {id, true};
	}

	template <typename T2>
	FindResult Find(const T2& value) const
	{
		if(m_Size == 0)
			return {INVALID_ID, 0, INVALID_ID};

		HashT baseHash = GetBaseHash(value);
		int hash = GetHash(baseHash);
		auto result = GetId(hash, value);
		if(result.id != INVALID_ID)
			return {result.id, int(hash), result.prevId};
		else
			return {INVALID_ID, 0, INVALID_ID};
	}

	EraseResult Erase(const FindResult& value)
	{
		if(!value.IsValid())
			return {false};

		if(value.id != m_Size - 1)
			m_Values[value.id] = std::move(m_Values[m_Size - 1]);
		m_Values[m_Size - 1].~T();
		m_Size--;

		m_Buckets[value.bucketId] = value.prevId != INVALID_ID ? m_NextIds[value.prevId] : INVALID_ID;
		if(value.prevId != INVALID_ID)
			m_NextIds[value.prevId] = INVALID_ID;

		return {true};
	}

	const T& GetValue(int id) const { return m_Values[id]; }
	T& GetValue(int id) { return m_Values[id]; }

	int GetSize() const { return m_Size; }
	int GetAllocated() const { return m_Allocated; }

	void Reserve(int count)
	{
		ReserveAndRehash(count);
	}

private:
	template <typename T2>
	HashT GetBaseHash(const T2& value) const
	{
		return m_Hasher(value);
	}
	int GetHash(HashT value) const
	{
		return int(value % m_BucketCount);
	}
	int CalculateNewSize(int minSize) const
	{
		int v;
		if(m_Size == 0)
			v = 8;
		else
			v = m_Size * 2;
		if(v <= m_Allocated)
			v = m_Allocated;
		if(v < minSize)
			v = minSize;
		return v;
	}
	int CalculateNewBucketCount(int minSize) const
	{
		int v;
		if(m_BucketCount == 0)
			v = 7;
		else
			v = CalculateNewSize(minSize);
		if(v < minSize)
			v = minSize;
		return v;
	}

	void AddToBucketList(HashT hash, int id)
	{
		int& curBucketValue = m_Buckets[hash];
		if(curBucketValue == INVALID_ID) {
			curBucketValue = id;
			m_NextIds[id] = INVALID_ID;
		} else {
			m_NextIds[id] = m_NextIds[curBucketValue];
			m_NextIds[curBucketValue] = id;
		}
	}

	void ReserveAndRehash(int minSize)
	{
		int newSize = CalculateNewSize(minSize);
		int newBucketCount = CalculateNewBucketCount(minSize);
		if(newSize > m_Allocated) {
			T* newValues = (T*)Alloc(sizeof(T) * newSize);
			for(int i = 0; i < m_Size; ++i) {
				new (newValues + i) T (std::move(m_Values[i]));
				m_Values[i].~T();
			}
			Free(m_Values);
			m_Values = newValues;

			int* newNext = (int*)Alloc(sizeof(int) * newSize);
			if(newBucketCount != m_BucketCount) {
				for(int i = 0; i < m_Size; ++i)
					newNext[i] = m_NextIds[i];
			}
			Free(m_NextIds);
			m_NextIds = newNext;

			m_Allocated = newSize;
		}

		if(newBucketCount != m_BucketCount) {
			Free(m_Buckets);
			m_Buckets = (int*)Alloc(sizeof(int)*newBucketCount);
			for(int i = 0; i < newBucketCount; ++i)
				m_Buckets[i] = INVALID_ID;
			m_BucketCount = newBucketCount;

			for(int i = 0; i < m_Size; ++i) {
				int hash = GetHash(m_Hasher(m_Values[i]));
				AddToBucketList(hash, i);
			}
		}
	}

	template <typename T2>
	IdResult GetId(HashT hash, const T2& value) const
	{
		int prevId = INVALID_ID;
		int curId = m_Buckets[hash];

		while(curId != INVALID_ID && !m_Comparer.Equal(m_Values[curId], value)) {
			prevId = curId;
			curId = m_NextIds[curId];
		}

		return {curId, prevId};
	}

	void* Alloc(int bytes)
	{
		return ::operator new(bytes);
	}
	void Free(void* ptr)
	{
		::operator delete(ptr);
	}

private:
	T* m_Values = nullptr;
	int* m_NextIds = nullptr;

	int m_Allocated = 0;
	int m_Size = 0;

	int* m_Buckets = nullptr;
	int m_BucketCount = 0;

	mutable HasherT m_Hasher;
	mutable ComparerT m_Comparer;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_BASIC_HASH_SET_H

