#ifndef INCLUDED_REFERENCECOUNTED_H
#define INCLUDED_REFERENCECOUNTED_H
#include "LuxBase.h"
#include <cstddef>
#include <vector>

namespace lux
{

template <typename T>
class WeakRef;

//! A object implementing reference counting
class ReferenceCounted
{
	template <typename T>
	friend class WeakRef;

private:
	mutable int m_ReferenceCounter;
	WeakRef<ReferenceCounted>* m_FirstWeak;

public:
	//! Create a new reference counting object
	inline ReferenceCounted();

	inline virtual ~ReferenceCounted();

	//! Reserve this object for me
	/**
	If you call this you reserve this object for you
	This object now can't be deleted before you called Drop() on it
	*/
	void Grab() const
	{
		++m_ReferenceCounter;
	}

	//! Remove ownership of this object
	/**
	If you call this you give up ownership of this object
	You _must_ call this sometime after a call to Grab()
	or when you retrieved this object from a method beginning with "create"
	for example createImage or createVertexBuffer...
	\return True if the object was fully deleted after this call
	*/
	bool Drop() const
	{
		assert(m_ReferenceCounter > 0);

		if(m_ReferenceCounter == 1) {
			LUX_FREE(this);
			return true;
		} else {
			--m_ReferenceCounter;
		}

		return false;
	}

	//! Get the actual reference count of this object
	/**
	\return The actual reference count of this object
	*/
	int GetReferenceCount() const
	{
		return m_ReferenceCounter;
	}
};

template <typename T>
class WeakRef;

template <typename T>
class StrongRef
{
	template <typename T2>
	friend class StrongRef;

private:
	ReferenceCounted* m_Object;

public:
	StrongRef() : m_Object(nullptr)
	{
	}

	StrongRef(T* obj) : m_Object(static_cast<ReferenceCounted*>(obj))
	{
		if(m_Object)
			m_Object->Grab();
	}

	StrongRef(const StrongRef<T>& other) : m_Object(other.m_Object)
	{
		if(m_Object)
			m_Object->Grab();
	}

	template <typename T2>
	StrongRef(const StrongRef<T2>& other) : m_Object(other.m_Object)
	{
		if(m_Object)
			m_Object->Grab();
	}

	StrongRef(const WeakRef<T>& other);

	template <typename T2>
	StrongRef(StrongRef<T2>&& old)
	{
		m_Object = old.m_Object;
		old.m_Object = nullptr;
	}

	StrongRef<T>& operator=(const StrongRef<T>& other)
	{
		*this = other.m_Object;
		return *this;
	}

	template <typename T2>
	StrongRef<T>& operator=(const StrongRef<T2>& other)
	{
		*this = dynamic_cast<T*>(other.m_Object);
		return *this;
	}

	template <typename T2>
	StrongRef<T>& operator=(StrongRef<T2>&& old)
	{
		if(m_Object)
			m_Object->Drop();

		m_Object = old.m_Object;
		old.m_Object = nullptr;

		return *this;
	}

	~StrongRef()
	{
		if(m_Object)
			m_Object->Drop();
	}

	template <typename T2>
	StrongRef<T>& operator=(T2* obj)
	{
		if(m_Object)
			m_Object->Drop();

		if(obj) {
			obj->Grab();
		}

		m_Object = obj;
		return *this;
	}

	T* operator->() const
	{
		if(!m_Object)
			return nullptr;
		T* casted_out =dynamic_cast<T*>(m_Object);
		assert(casted_out);
		return casted_out;
	}

	operator T*() const
	{
		if(!m_Object)
			return nullptr;
		T* casted_out =dynamic_cast<T*>(m_Object);
		assert(casted_out);
		return casted_out;
	}

	T* operator*() const
	{
		if(!m_Object)
			return nullptr;
		T* casted_out =dynamic_cast<T*>(m_Object);
		assert(casted_out);
		return casted_out;
	}

	WeakRef<T> MakeWeak();

	WeakRef<T> GetWeak() const;

	void Reset()
	{
		this->operator=(nullptr);
	}

	bool operator!() const
	{
		return !m_Object;
	}

	bool operator==(T* v) const
	{
		return m_Object == v;
	}

	bool operator==(nullptr_t) const
	{
		return m_Object == nullptr;
	}

	bool operator==(const StrongRef& other) const
	{
		return m_Object == other.m_Object;
	}

	bool operator!=(T* v) const
	{
		return m_Object != v;
	}

	bool operator!=(nullptr_t) const
	{
		return m_Object != nullptr;
	}

	bool operator!=(const StrongRef& other) const
	{
		return m_Object != other.m_Object;
	}

	template <typename T2>
	operator StrongRef<T2>() const
	{
		T2* cast_out = dynamic_cast<T2*>(m_Object);
		assert(cast_out);
		return StrongRef<T2>(cast_out);
	}

	template <typename T2>
	T2* As() const
	{
		return dynamic_cast<T2*>(m_Object);
	}
};

template <typename T>
class WeakRef
{
	friend class ReferenceCounted;
	template <typename T2>
	friend class WeakRef;

private:
	ReferenceCounted* m_Object;
	WeakRef<ReferenceCounted>* m_Next;
	WeakRef<ReferenceCounted>* m_Prev;

public:
	WeakRef() :
		m_Object(nullptr),
		m_Next(nullptr),
		m_Prev(nullptr)
	{
	}

	WeakRef(T* obj) :
		m_Object((T*)obj)
	{
		if(obj) {
			m_Prev = nullptr;
			if(obj->m_FirstWeak) {
				m_Next = obj->m_FirstWeak;
				obj->m_FirstWeak->m_Prev = (WeakRef<ReferenceCounted>*)this;
				obj->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
			} else {
				obj->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
				m_Next = nullptr;
			}
		} else {
			m_Prev = nullptr;
			m_Next = nullptr;
		}
	}

	WeakRef(const WeakRef<T>& other) : WeakRef(dynamic_cast<T*>(other.m_Object))
	{
	}

	WeakRef(const StrongRef<T>& other) : WeakRef(*other)
	{

	}

	WeakRef(WeakRef<T>&& old)
	{
		m_Object = old.m_Object;
		m_Prev = old.m_Prev;
		m_Next = old.m_Next;

		if(m_Prev)
			m_Prev->m_Next = (WeakRef<ReferenceCounted>*)this;
		if(m_Next)
			m_Next->m_Prev = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Object && old.m_Object->m_FirstWeak == (WeakRef<ReferenceCounted>*)(&old))
			old.m_Object->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
		old.m_Object = nullptr;
		old.m_Next = nullptr;
		old.m_Prev = nullptr;
	}

	WeakRef<T>& operator=(WeakRef<T>&& old)
	{
		if(m_Object) {
			if(m_Prev)
				m_Prev->m_Next = m_Next;
			else
				m_Object->m_FirstWeak = m_Next;

			if(m_Next)
				m_Next->m_Prev = m_Prev;
		}

		m_Object = old.m_Object;
		m_Prev = old.m_Prev;
		m_Next = old.m_Next;
		if(old.m_Prev)
			old.m_Prev->m_Next = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Next)
			old.m_Next->m_Prev = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Object && old.m_Object->m_FirstWeak == (WeakRef<ReferenceCounted>*)(&old))
			old.m_Object->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
		old.m_Object = nullptr;
		old.m_Next = nullptr;
		old.m_Prev = nullptr;

		return *this;
	}

	WeakRef<T>& operator=(const WeakRef<T>& other)
	{
		return this->operator=(dynamic_cast<T*>(other.m_Object));
	}

	template <typename T2>
	WeakRef(const WeakRef<T2>& other) : WeakRef(dynamic_cast<T2*>(other.m_Object))
	{
	}

	template <typename T2>
	WeakRef(const StrongRef<T2>& other) : WeakRef(dynamic_cast<T2*>(*other))
	{

	}

	template <typename T2>
	WeakRef(WeakRef<T2>&& old)
	{
		m_Object = old.m_Object;
		m_Prev = old.m_Prev;
		m_Next = old.m_Next;

		if(m_Prev)
			m_Prev->m_Next = (WeakRef<ReferenceCounted>*)this;
		if(m_Next)
			m_Next->m_Prev = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Object && old.m_Object->m_FirstWeak == (WeakRef<ReferenceCounted>*)(&old))
			old.m_Object->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
		old.m_Object = nullptr;
		old.m_Next = nullptr;
		old.m_Prev = nullptr;
	}

	template <typename T2>
	WeakRef<T>& operator=(WeakRef<T2>&& old)
	{
		if(m_Object) {
			if(m_Prev)
				m_Prev->m_Next = m_Next;
			else
				m_Object->m_FirstWeak = m_Next;

			if(m_Next)
				m_Next->m_Prev = m_Prev;
		}

		m_Object = old.m_Object;
		m_Prev = old.m_Prev;
		m_Next = old.m_Next;
		if(old.m_Prev)
			old.m_Prev->m_Next = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Next)
			old.m_Next->m_Prev = (WeakRef<ReferenceCounted>*)this;
		if(old.m_Object && old.m_Object->m_FirstWeak == (WeakRef<ReferenceCounted>*)(&old))
			old.m_Object->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
		old.m_Object = nullptr;
		old.m_Next = nullptr;
		old.m_Prev = nullptr;

		return *this;
	}

	template <typename T2>
	WeakRef<T>& operator=(const WeakRef<T2>& other)
	{
		return this->operator=(dynamic_cast<T*>(other.m_Object));
	}

	~WeakRef()
	{
		if(m_Object) {
			if(m_Prev)
				m_Prev->m_Next = m_Next;
			else
				m_Object->m_FirstWeak = m_Next;

			if(m_Next)
				m_Next->m_Prev = m_Prev;
		}
	}

	WeakRef<T>& operator=(T* obj)
	{
		if(m_Object) {
			if(m_Prev)
				m_Prev->m_Next = m_Next;
			else
				m_Object->m_FirstWeak = m_Next;

			if(m_Next)
				m_Next->m_Prev = m_Prev;
		}

		if(obj) {
			m_Prev = nullptr;
			if(obj->m_FirstWeak) {
				m_Next = obj->m_FirstWeak;
				obj->m_FirstWeak->m_Prev = (WeakRef<ReferenceCounted>*)this;
				obj->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
			} else {
				obj->m_FirstWeak = (WeakRef<ReferenceCounted>*)this;
				m_Next = nullptr;
			}
		} else {
			m_Prev = nullptr;
			m_Next = nullptr;
		}

		m_Object = obj;

		return *this;
	}

	T* operator->() const
	{
		return dynamic_cast<T*>(m_Object);
	}

	operator T*() const
	{
		return dynamic_cast<T*>(m_Object);
	}

	bool operator==(T* v) const
	{
		return m_Object == v;
	}

	bool operator==(nullptr_t) const
	{
		return m_Object == nullptr;
	}

	bool operator!=(T* v) const
	{
		return m_Object != v;
	}

	bool operator!=(nullptr_t) const
	{
		return m_Object != nullptr;
	}

	T* operator*() const
	{
		return dynamic_cast<T*>(m_Object);
	}

	StrongRef<T> GetStrong()
	{
		T* obj = dynamic_cast<T*>(m_Object);
		return StrongRef<T>(obj);
	}

	void Reset()
	{
		this->operator=(nullptr);
	}

	bool operator!() const
	{
		return !m_Object;
	}
};


ReferenceCounted::ReferenceCounted() :
	m_ReferenceCounter(0),
	m_FirstWeak(nullptr)
{
}

ReferenceCounted::~ReferenceCounted()
{
	auto weak = m_FirstWeak;
	while(weak) {
		auto next = weak->m_Next;
		weak->m_Next = nullptr;
		weak->m_Object = nullptr;
		weak->m_Prev = nullptr;
		weak = next;
	}
}

template <typename T>
WeakRef<T> StrongRef<T>::MakeWeak()
{
	T* obj = m_Object;
	*this = nullptr;
	return WeakRef<T>(obj);
}

template <typename T>
WeakRef<T> StrongRef<T>::GetWeak() const
{
	return WeakRef<T>(dynamic_cast<T*>(m_Object));
}

template <typename T>
StrongRef<T>::StrongRef(const WeakRef<T>& other) : StrongRef(*other)
{

}

}    


#endif