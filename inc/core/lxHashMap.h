#ifndef INCLUDED_LX_HASH_MAP_H
#define INCLUDED_LX_HASH_MAP_H
#include "lxAllocator.h"
#include "lxIterator.h"
#include "lxUtil.h"
#include "math/lxMath.h"

namespace lux
{
namespace core
{

template <typename K, typename V>
struct HashValue
{
	K key;
	V value;
	HashValue* next; // If the last bit in this pointer is set, the value doesn't have to be destructed
};

template <typename K, typename V,
	typename Hash = HashType<K>,
	typename KeyCompare = CompareType<K>,
	typename Alloc = Allocator<HashValue<K, V>>>
	class HashMap
{
public:
	class ConstIterator;

	class Iterator : public BaseIterator<core::ForwardIteratorTag, V>
	{
	public:
		Iterator() : m_Bucket(nullptr), m_Entry(nullptr)
		{
		}

		Iterator& operator++()
		{
			lxAssert(IsValidIterator());
			if(!m_Entry->next) {
				while((void*)*m_Bucket != (void*)m_Bucket) {
					++m_Bucket;
					if(*m_Bucket)
						break;
				}
				m_Entry = *m_Bucket;
			} else {
				m_Entry = m_Entry->next;
			}

			return *this;
		}
		Iterator operator++(int)
		{
			Iterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}
		bool operator==(const ConstIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const ConstIterator& other) const
		{
			return !(*this == other);
		}

		V& operator*()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}
		V* operator->()
		{
			lxAssert(IsValidIterator());
			return &(m_Entry->value);
		}

		V& value()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}

		const K& key()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}

	private:
		bool IsValidIterator() const
		{
			return (void*)*m_Bucket != (void*)m_Bucket;
		}

		explicit Iterator(HashValue<K, V>** b, HashValue<K, V>* e) : m_Bucket(b), m_Entry(e)
		{
		}
		explicit Iterator(HashValue<K, V>** b) : m_Bucket(b), m_Entry(b ? *b : nullptr)
		{
		}
		friend class HashMap<K, V, Hash, KeyCompare, Alloc>;
		friend class ConstIterator;

	private:
		HashValue<K, V>** m_Bucket;
		HashValue<K, V>* m_Entry;
	};

	class ConstIterator : public BaseIterator<core::ForwardIteratorTag, V>
	{
	public:
		ConstIterator() : m_Bucket(nullptr), m_Entry(nullptr)
		{
		}
		ConstIterator(const Iterator& iter) : m_Bucket(iter.m_Bucket), m_Entry(iter.m_Entry)
		{
		}

		ConstIterator& operator++()
		{
			lxAssert(IsValidIterator());
			if(!m_Entry->next) {
				while((void*)*m_Bucket != (void*)m_Bucket) {
					++m_Bucket;
					if(*m_Bucket)
						break;
				}
				m_Entry = *m_Bucket;
			} else {
				m_Entry = m_Entry->next;
			}

			return *this;
		}
		ConstIterator  operator++(int)
		{
			ConstIterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}
		bool operator==(const ConstIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const ConstIterator& other) const
		{
			return !(*this == other);
		}

		const V& operator*()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}
		const V* operator->()
		{
			lxAssert(IsValidIterator());
			return &(m_Entry->value);
		}

		const V& value()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}

		const K& key()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}

		ConstIterator& operator=(const Iterator& iter)
		{
			m_Bucket = iter.m_Bucket;
			m_Entry = iter.m_Entry;
			return *this;
		}

	private:
		bool IsValidIterator() const
		{
			return (void*)*m_Bucket != (void*)m_Bucket;
		}

		explicit ConstIterator(HashValue<K, V>** b, HashValue<K, V>* e) : m_Bucket(b), m_Entry(e)
		{
		}
		explicit ConstIterator(HashValue<K, V>** b) : m_Bucket(b), m_Entry(b ? *b : nullptr)
		{
		}

		friend class Iterator;
		friend class HashMap<K, V, Hash, KeyCompare, Alloc>;
	private:
		HashValue<K, V>** m_Bucket;
		HashValue<K, V>* m_Entry;
	};

	class ConstKeyIterator;

	class KeyIterator : public BaseIterator<core::ForwardIteratorTag, K>
	{
	public:
		KeyIterator()
		{
		}

		KeyIterator& operator++()
		{
			lxAssert(IsValidIterator());
			if(!m_Entry->next) {
				while((void*)*m_Bucket != (void*)m_Bucket) {
					++m_Bucket;
					if(*m_Bucket)
						break;
				}
				m_Entry = *m_Bucket;
			} else {
				m_Entry = m_Entry->next;
			}

			return *this;
		}
		KeyIterator operator++(int)
		{
			KeyIterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(const KeyIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const KeyIterator& other) const
		{
			return !(*this == other);
		}
		bool operator==(const ConstKeyIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const ConstKeyIterator& other) const
		{
			return !(*this == other);
		}

		K& operator*()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}
		K* operator->()
		{
			lxAssert(IsValidIterator());
			return &(m_Entry->key);
		}

		const V& value()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}

		const K& key()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}

	private:
		bool IsValidIterator() const
		{
			return (void*)*m_Bucket != (void*)m_Bucket;
		}

		explicit KeyIterator(HashValue<K, V>** b, HashValue<K, V>* e) : m_Bucket(b), m_Entry(e)
		{
		}
		explicit KeyIterator(HashValue<K, V>** b) : m_Bucket(b), m_Entry(b ? *b : nullptr)
		{
		}
		friend class HashMap<K, V, Hash, KeyCompare, Alloc>;
		friend class ConstKeyIterator;

	private:
		HashValue<K, V>** m_Bucket;
		HashValue<K, V>* m_Entry;
	};

	class ConstKeyIterator : public BaseIterator<core::ForwardIteratorTag, K>
	{
	public:
		ConstKeyIterator() : m_Bucket(nullptr), m_Entry(nullptr)
		{
		}
		ConstKeyIterator(const KeyIterator& iter) : m_Bucket(iter.m_Bucket), m_Entry(iter.m_Entry)
		{
		}

		ConstKeyIterator& operator++()
		{
			lxAssert(IsValidIterator());
			if(!m_Entry->next) {
				while((void*)*m_Bucket != (void*)m_Bucket) {
					++m_Bucket;
					if(*m_Bucket)
						break;
				}
				m_Entry = *m_Bucket;
			} else {
				m_Entry = m_Entry->next;
			}

			return *this;
		}
		ConstKeyIterator operator++(int)
		{
			ConstKeyIterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(const KeyIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const KeyIterator& other) const
		{
			return !(*this == other);
		}
		bool operator==(const ConstKeyIterator& other) const
		{
			return m_Entry == other.m_Entry;
		}
		bool operator!=(const ConstKeyIterator& other) const
		{
			return !(*this == other);
		}

		const K& operator*()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}
		const K* operator->()
		{
			lxAssert(IsValidIterator());
			return &(m_Entry->key);
		}

		ConstKeyIterator& operator=(const KeyIterator& iter)
		{
			m_Bucket = iter.m_Bucket;
			m_Entry = iter.m_Entry;
			return *this;
		}

		const V& value()
		{
			lxAssert(IsValidIterator());
			return m_Entry->value;
		}

		const K& key()
		{
			lxAssert(IsValidIterator());
			return m_Entry->key;
		}

	private:
		bool IsValidIterator() const
		{
			return (void*)*m_Bucket != (void*)m_Bucket;
		}

		explicit ConstKeyIterator(HashValue<K, V>** b, HashValue<K, V>* e) : m_Bucket(b), m_Entry(e)
		{
		}
		explicit ConstKeyIterator(HashValue<K, V>** b) : m_Bucket(b), m_Entry(b ? *b : nullptr)
		{
		}

		friend class KeyIterator;
		friend class HashMap<K, V, Hash, KeyCompare, Alloc>;
	private:
		HashValue<K, V>** m_Bucket;
		HashValue<K, V>* m_Entry;
	};

public:
	typedef K KeyType;
	typedef V ValueType;
	typedef Alloc AllocatorType;
	typedef Hash HasherType;
	typedef KeyCompare CompareType;

public:
	HashMap(size_t bucketCount = 7, size_t reserve = 0, double maxLoad = 1.0) :
		m_Buckets(nullptr),
		m_BucketCount(0),
		m_FirstBucket(0),
		m_Nodes(nullptr),
		m_FirstUnusedNode(nullptr),
		m_Allocated(0),
		m_Entries(0),
		m_MaxLoad(1.0),
		m_Hasher(),
		m_Equal()
	{
		if(maxLoad < 0.0)
			m_MaxLoad = 1.0;
		else
			m_MaxLoad = maxLoad;
		Init(bucketCount, reserve);
	}

	HashMap(const HashMap& other) :
		m_Buckets(nullptr),
		m_BucketCount(0),
		m_FirstBucket(0),
		m_Nodes(nullptr),
		m_FirstUnusedNode(nullptr),
		m_Allocated(0),
		m_Entries(0),
		m_MaxLoad(1.0f),
		m_Hasher(),
		m_Equal()
	{
		*this = other;
	}

	HashMap(HashMap&& old) :
		m_Buckets(nullptr),
		m_BucketCount(0),
		m_FirstBucket(0),
		m_Nodes(nullptr),
		m_FirstUnusedNode(nullptr),
		m_Allocated(0),
		m_Entries(0),
		m_MaxLoad(1.0f),
		m_Hasher(),
		m_Equal()
	{
		*this = std::move(old);
	}

	~HashMap()
	{
		Destroy();
	}

	HashMap& operator=(HashMap&& old)
	{
		this->Destroy();

		this->m_Buckets = old.m_Buckets;
		this->m_BucketCount = old.m_BucketCount;
		this->m_FirstBucket = old.m_FirstBucket;
		this->m_Nodes = old.m_Nodes;
		this->m_FirstUnusedNode = old.m_FirstUnusedNode;
		this->m_Allocated = old.m_Allocated;
		this->m_Entries = old.m_Entries;
		this->m_MaxLoad = old.m_MaxLoad;

		old.m_Buckets = nullptr;
		//old.m_BucketCount;
		//old.m_FirstBucket;
		old.m_Nodes = nullptr;
		old.m_FirstUnusedNode = nullptr;
		old.m_Allocated = 0;
		old.m_Entries = 0;
		//old.m_MaxLoad;

		old.Init(old.m_BucketCount, 0);

		return *this;
	}

	HashMap& operator=(const HashMap& other)
	{
		this->Destroy();

		this->m_MaxLoad = other.m_MaxLoad;
		this->Init(other.m_BucketCount, other.m_Allocated);

		for(size_t i = 0; i < this->m_Allocated; ++i) {
			HashValue<K, V>& newNode = this->m_Nodes[i];
			HashValue<K, V>& oldNode = other.m_Nodes[i];
			if(!PointerAnd(oldNode.next, 1)) {
				new ((void*)(&newNode.key)) K(oldNode.key);
				new ((void*)(&newNode.value)) V(oldNode.value);

				newNode.next = oldNode.next ?
					this->m_Nodes + (oldNode.next - other.m_Nodes) :
					nullptr;
			}
		}

		for(size_t i = 0; i < this->m_BucketCount; ++i) {
			this->m_Buckets[i] = other.m_Buckets[i] ?
				this->m_Nodes + (other.m_Buckets[i] - other.m_Nodes) :
				nullptr;
		}

		this->m_Entries = other.m_Entries;
		this->m_FirstBucket = other.m_FirstBucket;

		return *this;
	}

	template <typename Alloc2>
	bool operator==(const HashMap<K, V, Hash, KeyCompare, Alloc2>& other) const
	{
		if(this->Size() != other->Size())
			return false;
		// IDEA: If the maps have the same bucket count,
		// compare the individual buckets.

		for(auto it = this->cBegin(); this->cEnd(); ++it) {
			auto entry = other.Find(it.key());
			if(!m_Equal.Equal(it.value(), entry.value()))
				return false;
		}

		return true;
	}

	template <typename Alloc2>
	bool operator!=(const HashMap<K, V, Hash, KeyCompare, Alloc2>& other) const
	{
		return !(*this == other);
	}

	bool Set(const K& key, const V& value)
	{
		Iterator it;
		bool isNewEntry = FindOrAddEntry(key, it);
		if(isNewEntry)
			new ((void*)&it.m_Entry->value) V(value);
		else
			it.m_Entry->value = value;
		return isNewEntry;
	}

	Iterator Find(const K& key)
	{
		return FindEntry(key);
	}

	ConstIterator Find(const K& key) const
	{
		return FindEntry(key);
	}

	Iterator Erase(const K& key)
	{
		auto it = Find(key);
		if(it == End())
			return it;
		return Erase(it);
	}

	Iterator Erase(const Iterator& it)
	{
		HashValue<K, V>** bucket = it.m_Bucket;
		if(!bucket || bucket == m_Buckets + m_BucketCount)
			return it;

		Iterator next(it);
		++next;

		HashValue<K, V>* prev = nullptr;
		HashValue<K, V>* entry = *bucket;

		while(entry != it.m_Entry) {
			prev = entry;
			entry = entry->next;
		}

		if(prev)
			prev->next = entry->next;
		else
			*bucket = entry->next;

		DeleteEntry(entry);
		m_Allocator.Destruct(entry);
		m_Entries--;

		if(m_Buckets[m_FirstBucket] == *it.m_Bucket && it.m_Bucket != next.m_Bucket)
			m_FirstBucket = next.m_Bucket - m_Buckets;

		return next;
	}

	ConstIterator First() const
	{
		return FirstC();
	}

	ConstIterator End() const
	{
		return EndC();
	}

	ConstIterator FirstC() const
	{
		return ConstIterator(m_Buckets + m_FirstBucket);
	}

	ConstIterator EndC() const
	{
		return ConstIterator(m_Buckets + m_BucketCount);
	}

	Iterator First()
	{
		return Iterator(m_Buckets + m_FirstBucket);
	}

	Iterator End()
	{
		return Iterator(m_Buckets + m_BucketCount);
	}

	ConstKeyIterator FirstKey() const
	{
		return FirstKeyC();
	}

	ConstKeyIterator EndKey() const
	{
		return EndKeyC();
	}

	ConstKeyIterator FirstKeyC() const
	{
		return ConstKeyIterator(m_Buckets + m_FirstBucket);
	}

	ConstKeyIterator EndKeyC() const
	{
		return ConstKeyIterator(m_Buckets + m_BucketCount);
	}

	KeyIterator FirstKey()
	{
		return KeyIterator(m_Buckets + m_FirstBucket);
	}

	KeyIterator EndKey()
	{
		return KeyIterator(m_Buckets + m_BucketCount);
	}

	size_t Size() const
	{
		return m_Entries;
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
		m_FirstUnusedNode = nullptr;
		for(size_t i = 0; i < m_Allocated; ++i) {
			if(!PointerAnd(m_Nodes[i].next, 1)) {
				m_Allocator.Destruct(m_Nodes + i);

				m_Nodes[i].next = PointerOr(m_FirstUnusedNode, 1);
				m_FirstUnusedNode = m_Nodes + i;
			}
		}

		for(size_t i = 0; i < m_BucketCount; ++i)
			m_Buckets[i] = nullptr;

		m_Entries = 0;
		m_FirstBucket = m_BucketCount;
	}

	void Reserve(size_t newSize)
	{
		if(newSize > m_Allocated)
			ResizeAndRehash(m_BucketCount, newSize);
	}

	V& operator[](const K& key)
	{
		return At(key);
	}
	const V& operator[](const K& key) const
	{
		return At(key);
	}

	V& At(const K& key)
	{
		Iterator it;
		if(FindOrAddEntry(key, it))
			new ((void*)&it.m_Entry->value) V();
		return it.m_Entry->value;
	}
	V& At(const K& key, const V& defaultValue)
	{
		Iterator it;
		if(FindOrAddEntry(key, it))
			new ((void*)&it.m_Entry->value) V(defaultValue);
		return it.m_Entry->value;
	}
	const V& At(const K& key) const
	{
		return FindEntry(key).m_Entry->value;
	}
	const V& At(const K& key, const V& defaultValue) const
	{
		Iterator it = FindEntry(key);
		if(it == End())
			return defaultValue;
		return it.m_Entry->value;
	}

	void Rehash(size_t newBucketCount)
	{
		ResizeAndRehash(newBucketCount, m_Allocated);
	}

	void SetMaxLoad(double maxLoad)
	{
		if(maxLoad >= 0.0) {
			m_MaxLoad = maxLoad;
			ResizeAndRehash(m_BucketCount, m_Allocated);
		}
	}

	double GetMaxLoad() const
	{
		return m_MaxLoad;
	}

	double GetLoad() const
	{
		if(m_Buckets != 0)
			return (double)m_Entries / (double)m_Buckets;
		else
			return 0.0;
	}

	const Hash& Hasher() const
	{
		return m_Hasher;
	}

	const KeyCompare& Comparator() const
	{
		return m_Equal;
	}

	Alloc& Allocator()
	{
		return m_Allocator;
	}

	//! Support for foreach loop
	Iterator begin()
	{
		return Iterator(m_Buckets + m_FirstBucket);
	}

	//! Support for foreach loop
	Iterator end()
	{
		return Iterator(m_Buckets + m_BucketCount);
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

	struct ValueAccess
	{
		HashMap& ref;
		ValueAccess(HashMap& r) :
			ref(r)
		{
		}

		//! Support for foreach loop
		Iterator begin()
		{
			return ref.begin();
		}

		//! Support for foreach loop
		Iterator end()
		{
			return ref.end();
		}

		//! Support for foreach loop
		ConstIterator begin() const
		{
			return ref.begin();
		}

		//! Support for foreach loop
		ConstIterator end() const
		{
			return ref.end();
		}
	};

	struct ConstValueAccess
	{
		const HashMap& ref;
		ConstValueAccess(const HashMap& r) :
			ref(r)
		{
		}

		//! Support for foreach loop
		Iterator begin()
		{
			return ref.begin();
		}

		//! Support for foreach loop
		Iterator end()
		{
			return ref.end();
		}

		//! Support for foreach loop
		ConstIterator begin() const
		{
			return ref.begin();
		}

		//! Support for foreach loop
		ConstIterator end() const
		{
			return ref.end();
		}
	};

	ValueAccess Values()
	{
		return ValueAccess(*this);
	}

	ConstValueAccess Values() const
	{
		return ConstValueAccess(*this);
	}

	struct KeyAccess
	{
		HashMap& ref;
		KeyAccess(HashMap& r) :
			ref(r)
		{
		}

		//! Support for foreach loop
		Iterator begin()
		{
			return ref.FirstKey();
		}

		//! Support for foreach loop
		Iterator end()
		{
			return ref.EndKey();
		}

		//! Support for foreach loop
		ConstIterator begin() const
		{
			return ref.FirstKey();
		}

		//! Support for foreach loop
		ConstIterator end() const
		{
			return ref.EndKey();
		}
	};

	struct ConstKeyAccess
	{
		const HashMap& ref;
		ConstKeyAccess(const HashMap& r) :
			ref(r)
		{
		}

		//! Support for foreach loop
		ConstIterator begin() const
		{
			return ref.FirstKey();
		}

		//! Support for foreach loop
		ConstIterator end() const
		{
			return ref.EndKey();
		}
	};

	KeyAccess Keys()
	{
		return KeyAccess(*this);
	}

	ConstKeyAccess Keys() const
	{
		return ConstKeyAccess(*this);
	}

private:
	template <typename T>
	static T* PointerAnd(T* ptr, size_t v)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) & v);
	}

	template <typename T>
	static T* PointerOr(T* ptr, size_t v)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) | v);
	}

	void Init(size_t bucketCount, size_t reserved)
	{
		ResizeAndRehash(bucketCount, reserved);
	}

	void Destroy()
	{
		m_BucketAllocator.Deallocate(m_Buckets);

		for(size_t i = 0; i < m_Allocated; ++i) {
			if(!PointerAnd(m_Nodes[i].next, 1))
				m_Allocator.Destruct(m_Nodes + i);
		}
		m_Allocator.Deallocate(m_Nodes);

		m_Buckets = nullptr;
		m_BucketCount = 0;
		m_FirstBucket = 0;
		m_Nodes = nullptr;
		m_FirstUnusedNode = nullptr;
		m_Allocated = 0;
		m_Entries = 0;
	}

	// IDEA: Improve this function, by keeping the old node list if possible
	void ResizeAndRehash(size_t newBucketCount, size_t newSize)
	{
		if(!m_Buckets) {
			const size_t minBucketCount = static_cast<size_t>(newSize / m_MaxLoad);
			if(minBucketCount > newBucketCount)
				newBucketCount = minBucketCount;

			if(newBucketCount != m_BucketCount)
				newBucketCount = math::NextPrime(newBucketCount);

			m_Buckets = m_BucketAllocator.Allocate(newBucketCount + 1);
			for(size_t i = 0; i < newBucketCount + 1; ++i)
				m_BucketAllocator.Construct(m_Buckets + i);
			m_Buckets[newBucketCount] =
				reinterpret_cast<HashValue<K, V>*>(m_Buckets + newBucketCount);

			m_FirstBucket = newBucketCount;
			m_BucketCount = newBucketCount;
		}

		const size_t minBucketCount = (size_t)(newSize / m_MaxLoad) + 1;
		if(minBucketCount > newBucketCount)
			newBucketCount = minBucketCount;

		if(newBucketCount != m_BucketCount)
			newBucketCount = math::NextPrime(newBucketCount);

		if(newBucketCount != m_BucketCount) {
			HashMap newMap(newBucketCount, newSize, this->m_MaxLoad);
			for(auto it = this->First(); it != this->End(); ++it)
				newMap.Set(it.key(), it.value());
			this->Destroy();
			this->m_Allocated = newMap.m_Allocated;
			this->m_BucketCount = newMap.m_BucketCount;
			this->m_Buckets = newMap.m_Buckets;
			this->m_Entries = newMap.m_Entries;
			this->m_FirstBucket = newMap.m_FirstBucket;
			this->m_FirstUnusedNode = newMap.m_FirstUnusedNode;
			this->m_MaxLoad = newMap.m_MaxLoad;
			this->m_Nodes = newMap.m_Nodes;
			newMap.m_Nodes = nullptr;
			newMap.m_Allocated = 0;
			newMap.m_BucketCount = 0;
			newMap.m_Buckets = nullptr;
			newMap.m_Entries = 0;
		} else {
			ResizeEntries(newSize);
		}
	}

	void ResizeEntries(size_t newSize)
	{
		static_assert(sizeof(HashValue<K, V>) % 2 == 0,
			"Hash Value has a byte-size, shouldn't happen");

		if(newSize > m_Allocated) {
			HashValue<K, V>* newNodes = m_Allocator.Allocate(newSize);
			lxAssert((size_t)newNodes % 2 == 0);
			if(m_Allocated) {
				for(size_t i = 0; i < m_BucketCount; ++i) {
					if(m_Buckets[i])
						m_Buckets[i] = newNodes + (m_Buckets[i] - m_Nodes);
				}
			}

			m_FirstUnusedNode = nullptr;
			for(size_t i = 0; i < m_Allocated; ++i) {
				HashValue<K, V>& newNode = newNodes[i];
				HashValue<K, V>& oldNode = m_Nodes[i];
				if(!PointerAnd(oldNode.next, 1)) {
					new ((void*)(&newNodes[i].key)) K(std::move(oldNode.key));
					new ((void*)(&newNodes[i].value)) V(std::move(oldNode.value));

					newNodes[i].next = oldNode.next ?
						newNodes + (oldNode.next - m_Nodes) :
						nullptr;
					m_Allocator.Destruct(m_Nodes + i);
				} else {
					newNode.next = PointerOr(m_FirstUnusedNode, 1);
					m_FirstUnusedNode = newNodes + i;
				}
			}

			for(size_t i = m_Allocated; i < newSize; ++i) {
				newNodes[i].next = PointerOr(m_FirstUnusedNode, 1);
				m_FirstUnusedNode = newNodes + i;
			}

			m_Allocator.Deallocate(m_Nodes);
			m_Nodes = newNodes;
			m_Allocated = newSize;
		}
	}

	HashValue<K, V>* NewEntry()
	{
		if(!m_FirstUnusedNode) {
			const size_t newCount = (m_Allocated * 3) / 2 + 1;
			Reserve(newCount);
		}

		if(double(m_Entries + 1) / m_BucketCount > GetMaxLoad())
			ResizeAndRehash((m_BucketCount * 3) / 2 + 1, m_Allocated);

		HashValue<K, V>* out = m_FirstUnusedNode;
		m_FirstUnusedNode = PointerAnd(m_FirstUnusedNode->next, ~(size_t)1);
		return out;
	}

	void DeleteEntry(HashValue<K, V>* entry)
	{
		entry->next = PointerOr(m_FirstUnusedNode, 1);
		m_FirstUnusedNode = entry;
	}

	Iterator FindEntry(const K& key) const
	{
		if(m_Entries == 0)
			return Iterator(m_Buckets + m_BucketCount);

		const size_t hash = m_Hasher(key) % m_BucketCount;
		HashValue<K, V>* entry = m_Buckets[hash];

		while(entry && !m_Equal.Equal(entry->key, key))
			entry = entry->next;

		if(!entry)
			return Iterator(m_Buckets + m_BucketCount);
		else
			return Iterator(m_Buckets + hash, entry);
	}

	bool FindOrAddEntry(const K& key, Iterator& newIter)
	{
		const size_t pureHash = m_Hasher(key);
		size_t hash = pureHash % m_BucketCount;
		HashValue<K, V>* prev = nullptr;
		HashValue<K, V>* entry = m_Buckets[hash];

		while(entry && !m_Equal.Equal(entry->key, key)) {
			prev = entry;
			entry = entry->next;
		}

		const bool isNewEntry = (entry == nullptr);
		if(isNewEntry) {
			const size_t oldBucketCount = m_BucketCount;
			const size_t oldAllocated = m_Allocated;

			entry = NewEntry();
			new ((void*)&entry->key) K(key);
			entry->next = nullptr;

			// There was a rehash => recalculate data
			if(oldBucketCount != m_BucketCount ||
				oldAllocated != m_Allocated) {
				hash = pureHash % m_BucketCount;
				if(prev) {
					prev = nullptr;
					HashValue<K, V>* e = m_Buckets[hash];
					while(e && !m_Equal.Equal(e->key, key)) {
						prev = e;
						e = e->next;
					}
				}
			}

			if(prev)
				prev->next = entry;
			else {
				HashValue<K, V>* e = m_Buckets[hash];
				m_Buckets[hash] = entry;
				entry->next = e;
			}

			++m_Entries;
			if(hash < m_FirstBucket)
				m_FirstBucket = hash;
		}

		newIter = Iterator(m_Buckets + hash, entry);
		return isNewEntry;
	}

private:
	HashValue<K, V>** m_Buckets;
	size_t m_BucketCount;
	size_t m_FirstBucket;

	HashValue<K, V>* m_Nodes;
	HashValue<K, V>* m_FirstUnusedNode;
	size_t m_Allocated;

	size_t m_Entries;

	double m_MaxLoad;

	const Hash m_Hasher;
	const KeyCompare m_Equal;

	Alloc m_Allocator;
	core::Allocator<HashValue<K, V>*> m_BucketAllocator;
};

}
}

#endif // !INCLUDED_LX_HASH_MAP_H
