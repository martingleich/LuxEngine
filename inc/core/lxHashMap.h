#ifndef INCLUDED_LUX_LX_HASH_MAP_H
#define INCLUDED_LUX_LX_HASH_MAP_H
#include "core/BasicHashSet.h"

namespace lux
{
namespace core
{

template <typename K, typename V, typename HasherT = HashType<K>, typename ComparerT = CompareType<K>>
class HashMap
{
	struct RefTuple
	{
		const K& key;
		const V& value;
		RefTuple(const K& k, const V& v) :
			key(k),
			value(v)
		{
		}
	};
	struct DefaultTuple
	{
		const K& key;
		DefaultTuple(const K& k) :
			key(k)
		{
		}
	};
	template <typename CallT>	
	struct CallTuple
	{
		CallTuple(const K& key, CallT& call) :
			m_Call(call),
			m_Key(key)
		{
		}
		CallT& m_Call;
		const K& m_Key;
	};
	struct Tuple
	{
		K key;
		V value;

		Tuple(const RefTuple& ref) :
			key(ref.key),
			value(ref.value)
		{
		}
		Tuple(const DefaultTuple& ref) :
			key(ref.key)
		{
		}
		template <typename CallT>
		Tuple(const CallTuple<CallT>& ref) :
			key(ref.m_Key),
			value(ref.m_Call())
		{}
		Tuple(const K& k, const V& v) :
			key(k),
			value(v)
		{
		}
		Tuple(const Tuple&) = default;
		Tuple(Tuple&& old) = default;
		Tuple& operator=(const Tuple&) = default;
		Tuple& operator=(Tuple&& old) = default;
		Tuple& operator=(const RefTuple& ref)
		{
			key = ref.key;
			value = ref.value;
			return *this;
		}
		Tuple& operator=(const DefaultTuple& ref)
		{
			key = ref.key;
			value = V();
			return *this;
		}
	};
	struct TupleHasher
	{
		HasherT hasher;
		TupleHasher() {}
		TupleHasher(const HasherT& _hasher) :
			hasher(_hasher)
		{
		}
		unsigned int operator()(const Tuple& e)
		{
			return hasher(e.key);
		}
		template <typename KeyT>
		unsigned int operator()(const KeyT& key)
		{
			return hasher(key);
		}
	};
	struct TupleComparer
	{
		ComparerT comparer;
		TupleComparer() {}
		TupleComparer(const ComparerT& _comparer) :
			comparer(_comparer)
		{
		}
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

	using BaseType = BasicHashSet<Tuple, TupleHasher, TupleComparer>;

	class KeyNotFoundException : public ErrorException
	{
	public:
		KeyNotFoundException()
		{
		}
		ExceptionSafeString What() const override
		{
			return "Key not found";
		}
	};
public:
	struct TupleItState
	{
		TupleItState(const Tuple* t) : ptr(const_cast<Tuple*>(t)) {}
		void next() { ++ptr; }
		void prev() { ++ptr; }
		const Tuple& get_const() const { return *ptr; }
		Tuple& get_ref() { return *ptr; }
		bool cmp(TupleItState other) const { return ptr == other.ptr; }
		Tuple* ptr;
	};
	LX_MAKE_BASE_BI_ITER(Iterator, TupleItState, Tuple);
	struct KeyItState
	{
		KeyItState(const Tuple* t) : ptr(const_cast<Tuple*>(t)) {}
		void next() { ++ptr; }
		void prev() { ++ptr; }
		const K& get_const() const { return ptr->key; }
		K& get_ref() { return ptr->key; }
		bool cmp(KeyItState other) const { return ptr == other.ptr; }
		Tuple* ptr;
	};
	LX_MAKE_BASE_BI_ITER(KeyIterator, KeyItState, K);
	struct ValueItState
	{
		ValueItState(const Tuple* t) : ptr(const_cast<Tuple*>(t)) {}
		void next() { ++ptr; }
		void prev() { ++ptr; }
		const V& get_const() const { return ptr->value; }
		V& get_ref() { return ptr->value; }
		bool cmp(ValueItState other) const { return ptr == other.ptr; }
		Tuple* ptr;
	};
	LX_MAKE_BASE_BI_ITER(ValueIterator, ValueItState, V);

	using KeyT = K;
	using ValueT = V;

	struct SetResult
	{
		Iterator it;
		bool addedNew;

		const V& GetValue() const
		{
			return it->value;
		}
	};

	struct EraseResult
	{
		bool removed;
	};
	
public:
	HashMap() = default;
	HashMap(const HashMap& other) = default;
	HashMap(HashMap&& old) = default;
	HashMap& operator=(const HashMap& other) = default;
	HashMap& operator=(HashMap&& old) = default;
	~HashMap() = default;

	// Resizing
	void Reserve(int allocated) { m_Base.Reserve(allocated); }
	void Clear() { m_Base.Clear(); }

	SetResult SetAndReplace(const K& key, const V& value)
	{
		auto result = m_Base.Add(key, BaseType::EAddOption::Replace, RefTuple(key, value));
		return {Iterator(&m_Base.GetValue(result.id)), result.addedNew};
	}
	SetResult SetIfNotExist(const K& key, const V& value)
	{
		auto result = m_Base.Add(key, BaseType::EAddOption::FailOnDuplicate, RefTuple(key, value));
		return {Iterator(&m_Base.GetValue(result.id)), result.addedNew};
	}
	template <typename CallT>
	SetResult MakeIfNotExist(const K& key, CallT&& call)
	{
		auto result = m_Base.Add(key, BaseType::EAddOption::FailOnDuplicate, CallTuple<CallT>(key, call));
		return {Iterator(&m_Base.GetValue(result.id)), result.addedNew};
	}

	template <typename K2 = K>
	bool HasKey(const K2& key) const
	{
		return m_Base.Find<K2>(key).IsValid();
	}

	template <typename K2 = K>
	core::Optional<Iterator> Find(const K2& key)
	{
		auto result = m_Base.Find(key);
		if(!result.IsValid())
			return core::Optional<Iterator>();
		return Iterator(&m_Base.GetValue(result.id));
	}

	template <typename K2 = K>
	core::Optional<ConstIterator> Find(const K2& key) const
	{
		auto result = m_Base.Find(key);
		if(!result.IsValid())
			return core::Optional<ConstIterator>();
		return ConstIterator(&m_Base.GetValue(result.id));
	}

	template <typename K2 = K>
	EraseResult Erase(const K2& key)
	{
		auto result = m_Base.Find(key);
		return {m_Base.Erase(result).removed};
	}

	void EraseIter(Iterator it)
	{
		Erase(*it);
	}

	V& operator[](const K& key) { return At(key); }
	const V& operator[](const K& key) const { return Get(key); }

	template <typename K2=K>
	V& At(const K2& key, const V& init)
	{
		return m_Base.GetValue(m_Base.Add(key, BaseType::EAddOption::FailOnDuplicate, RefTuple(key, init)).id).value;
	}

	template <typename K2 = K>
	V& At(const K2& key)
	{
		auto result = m_Base.Add(key, BaseType::EAddOption::FailOnDuplicate, DefaultTuple(key));
		return m_Base.GetValue(result.id).value;
	}

	template <typename K2 = K>
	const V& Get(const K2& key) const
	{
		auto result = m_Base.Find(key);
		if(!result.IsValid())
			throw KeyNotFoundException();
		return m_Base.GetValue(result.id).value;
	}

	template <typename K2 = K>
	const V& Get(const K2& key, const V& def) const
	{
		auto result = m_Base.Find(key);
		if(result.IsValid())
			return m_Base.GetValue(result.id).value;
		else
			return def;
	}

	// Iterators
	ConstIterator begin() const { return ConstIterator(&m_Base.GetValue(0)); }
	ConstIterator end() const { return ConstIterator(&m_Base.GetValue(Size())); }

	Iterator begin() { return Iterator(&m_Base.GetValue(0)); }
	Iterator end() { return Iterator(&m_Base.GetValue(Size())); }

	core::Range<KeyIterator> Keys() { return core::MakeRange(KeyIterator(&m_Base.GetValue(0)), KeyIterator(&m_Base.GetValue(Size()))); }
	core::Range<ConstKeyIterator> Keys() const { return core::MakeRange(ConstKeyIterator(&m_Base.GetValue(0)), ConstKeyIterator(&m_Base.GetValue(Size()))); }
	core::Range<ValueIterator> Values() { return core::MakeRange(ValueIterator(&m_Base.GetValue(0)), ValueIterator(&m_Base.GetValue(Size()))); }
	core::Range<ConstValueIterator> Values() const { return core::MakeRange(ConstValueIterator(&m_Base.GetValue(0)), ConstValueIterator(&m_Base.GetValue(Size()))); }

	// Infomations
	int Size() const { return m_Base.GetSize(); }
	int Allocated() const { return m_Base.GetAllocated(); }
	bool IsEmpty() const { return m_Base.GetSize() == 0; }

private:
	BaseType m_Base;
};

template <typename K, typename V, typename HasherT, typename ComparerT>
typename HashMap<K, V, HasherT, ComparerT>::Iterator begin(HashMap<K, V, HasherT, ComparerT>& map) { return map.begin(); }
template <typename K, typename V, typename HasherT, typename ComparerT>
typename HashMap<K, V, HasherT, ComparerT>::Iterator end(HashMap<K, V, HasherT, ComparerT>& map) { return map.end(); }

template <typename K, typename V, typename HasherT, typename ComparerT>
typename HashMap<K, V, HasherT, ComparerT>::ConstIterator begin(const HashMap<K, V, HasherT, ComparerT>& map) { return map.begin(); }
template <typename K, typename V, typename HasherT, typename ComparerT>
typename HashMap<K, V, HasherT, ComparerT>::ConstIterator end(const HashMap<K, V, HasherT, ComparerT>& map) { return map.end(); }

} // namespace core
} // namespace lux

#endif // !INCLUDED_LUX_LX_HASH_MAP_H
