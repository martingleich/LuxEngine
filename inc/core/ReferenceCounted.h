#ifndef INCLUDED_LUX_REFERENCECOUNTED_H
#define INCLUDED_LUX_REFERENCECOUNTED_H
#include "LuxBase.h"
#include "lxException.h"
#include <cstddef>

namespace lux
{

class ReferenceCounted;

namespace core
{
class RefCountedObserver
{
	friend class ReferenceCounted;
public:
	RefCountedObserver() :
		m_Next(nullptr),
		m_Prev(nullptr)
	{
	}
	RefCountedObserver(RefCountedObserver* n, RefCountedObserver* p) :
		m_Next(n),
		m_Prev(p)
	{
	}

	virtual ~RefCountedObserver()
	{
	}

protected:
	virtual void Destroy() = 0;

	void AddTo(ReferenceCounted* obj);
	void AssignTo(ReferenceCounted* from, ReferenceCounted* to);
	void RemoveFrom(ReferenceCounted* obj);

protected:
	RefCountedObserver* m_Next;
	RefCountedObserver* m_Prev;
};
}

//! A object implementing reference counting
class ReferenceCounted
{
	friend class core::RefCountedObserver;

private:
	mutable int m_ReferenceCounter;
	core::RefCountedObserver* m_FirstWeak;

public:
	//! Create a new reference counting object
	ReferenceCounted() :
		m_ReferenceCounter(0),
		m_FirstWeak(nullptr)
	{
	}

	//! Remove all referenced when copying a refernec counted object.
	ReferenceCounted(const ReferenceCounted&) :
		m_ReferenceCounter(0),
		m_FirstWeak(nullptr)
	{
	}

	//! Don't allow assignment.
	ReferenceCounted& operator=(const ReferenceCounted& other) = delete;

	virtual ~ReferenceCounted()
	{
	}

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
	You _must_ call this sometime after a call to Grab().
	\return True if the object was fully deleted after this call
	*/
	bool Drop() const;

	//! Get the actual reference count of this object
	/**
	\return The actual reference count of this object
	*/
	int GetReferenceCount() const
	{
		return m_ReferenceCounter;
	}
};

inline void core::RefCountedObserver::AddTo(ReferenceCounted* obj)
{
	if(obj) {
		m_Prev = nullptr;
		if(obj->m_FirstWeak) {
			m_Next = obj->m_FirstWeak;
			obj->m_FirstWeak->m_Prev = this;
			obj->m_FirstWeak = this;
		} else {
			obj->m_FirstWeak = this;
			m_Next = nullptr;
		}
	} else {
		m_Prev = nullptr;
		m_Next = nullptr;
	}
}

inline void core::RefCountedObserver::AssignTo(ReferenceCounted* from, ReferenceCounted* to)
{
	if(from) {
		if(m_Prev)
			m_Prev->m_Next = m_Next;
		else
			from->m_FirstWeak = m_Next;

		if(m_Next)
			m_Next->m_Prev = m_Prev;
	}

	if(to) {
		m_Prev = nullptr;
		if(to->m_FirstWeak) {
			m_Next = to->m_FirstWeak;
			to->m_FirstWeak->m_Prev = this;
			to->m_FirstWeak = this;
		} else {
			to->m_FirstWeak = this;
			m_Next = nullptr;
		}
	} else {
		m_Prev = nullptr;
		m_Next = nullptr;
	}
}

inline void core::RefCountedObserver::RemoveFrom(ReferenceCounted* obj)
{
	if(obj) {
		if(m_Prev)
			m_Prev->m_Next = m_Next;
		else
			obj->m_FirstWeak = m_Next;

		if(m_Next)
			m_Next->m_Prev = m_Prev;
	}
}

template <typename T>
class WeakRef;

template <typename T>
class StrongRef
{
	template <typename T2>
	friend class StrongRef;

private:
	T* m_Object;

public:
	StrongRef() :
		m_Object(nullptr)
	{
	}

	StrongRef(T* obj) :
		m_Object(obj)
	{
		if(m_Object)
			m_Object->Grab();
	}

	StrongRef(const StrongRef& other) :
		StrongRef(other.m_Object)
	{
	}

	template <typename T2, bool U = std::is_base_of<T, T2>::value, std::enable_if_t<U, int> = 0>
	StrongRef(const StrongRef<T2>& other) :
		StrongRef(other.m_Object)
	{
	}

	StrongRef(const WeakRef<T>& other);

	void Reset()
	{
		if(m_Object) {
			m_Object->Drop();
			m_Object = nullptr;
		}
	}

	~StrongRef()
	{
		Reset();
	}

	StrongRef<T>& operator=(T* obj)
	{
		if(obj)
			obj->Grab();

		if(m_Object)
			m_Object->Drop();

		m_Object = obj;
		return *this;
	}

	// Explicit to make since there are two possible casts( = (T*)x, oder = StrongRef(x))
	StrongRef<T>& operator=(std::nullptr_t)
	{
		Reset();
		return *this;
	}

	StrongRef& operator=(StrongRef&& old)
	{
		if(m_Object)
			m_Object->Drop();

		m_Object = old.m_Object;
		old.m_Object = nullptr;

		return *this;
	}

	template <typename T2>
	std::enable_if_t<std::is_base_of<T, T2>::value, StrongRef<T>&>
		operator=(StrongRef<T2>&& old)
	{
		if(m_Object)
			m_Object->Drop();

		m_Object = old.m_Object;
		old.m_Object = nullptr;

		return *this;
	}

	StrongRef& operator=(const StrongRef& other)
	{
		*this = other.m_Object;
		return *this;
	}

	template <typename T2>
	std::enable_if_t<std::is_base_of<T, T2>::value, StrongRef<T>&>
		operator=(const StrongRef<T2>& other)
	{
		*this = other.m_Object;
		return *this;
	}

	T* operator->() const
	{
		return m_Object;
	}

	operator T*() const
	{
		return m_Object;
	}

	T* Raw() const
	{
		return m_Object;
	}

	bool operator!() const
	{
		return !m_Object;
	}

	bool operator==(T* v) const
	{
		return m_Object == v;
	}

	bool operator==(std::nullptr_t) const
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

	bool operator!=(std::nullptr_t) const
	{
		return m_Object != nullptr;
	}

	bool operator!=(const StrongRef& other) const
	{
		return m_Object != other.m_Object;
	}

	template <typename T2>
	T2* As() const
	{
		return dynamic_cast<T2*>(m_Object);
	}

	template <typename T2>
	StrongRef<T2> AsStrong() const
	{
		return As<T2>();
	}

	template <typename T2>
	T2* StaticCast() const
	{
		return static_cast<T2*>(m_Object);
	}

	template <typename T2>
	StrongRef<T2> StaticCastStrong() const
	{
		return StaticCast<T2>();
	}

	WeakRef<T> GetWeak() const;
};

template <typename T>
class WeakRef : public core::RefCountedObserver
{
	template <typename T2>
	friend class WeakRef;

private:
	T* m_Object;

public:
	WeakRef() :
		m_Object(nullptr)
	{
	}

	WeakRef(T* obj) :
		m_Object(obj)
	{
		AddTo(obj);
	}

	WeakRef(const WeakRef& other) :
		WeakRef(other.m_Object)
	{
	}

	template <typename T2, bool U = std::is_base_of<T, T2>::value, std::enable_if_t<U, int> = 0>
	WeakRef(const WeakRef<T2>& other) :
		WeakRef(other.m_Object)
	{
	}

	void Reset()
	{
		if(m_Object) {
			RemoveFrom(m_Object);
			m_Object = nullptr;
		}
	}

	~WeakRef()
	{
		Reset();
	}

	WeakRef& operator=(const WeakRef& other)
	{
		return this->operator=(other.m_Object);
	}

	template <typename T2>
	std::enable_if_t<std::is_base_of<T, T2>::value, WeakRef<T>&>
		operator=(const WeakRef<T2>& other)
	{
		return this->operator=(other.m_Object);
	}

	WeakRef& operator=(T* obj)
	{
		AssignTo(m_Object, obj);
		m_Object = obj;

		return *this;
	}

	T* operator->() const
	{
		return m_Object;
	}

	operator T*() const
	{
		return m_Object;
	}

	T* Raw() const
	{
		return m_Object;
	}

	bool operator==(const WeakRef& v) const
	{
		return m_Object == v.m_Object;
	}

	bool operator!=(const WeakRef& v) const
	{
		return m_Object != v.m_Object;
	}

	bool operator==(T* v) const
	{
		return m_Object == v;
	}

	bool operator==(std::nullptr_t) const
	{
		return m_Object == nullptr;
	}

	bool operator!=(T* v) const
	{
		return m_Object != v;
	}

	bool operator!=(std::nullptr_t) const
	{
		return m_Object != nullptr;
	}

	template <typename T2>
	T2* As() const
	{
		return dynamic_cast<T2*>(m_Object);
	}

	StrongRef<T> GetStrong()
	{
		return m_Object;
	}

	bool operator!() const
	{
		return !m_Object;
	}

protected:
	void Destroy()
	{
		m_Object = nullptr;
	}
};

template <typename T>
StrongRef<T>::StrongRef(const WeakRef<T>& other) :
	StrongRef(other.Raw())
{
}

template <typename T>
WeakRef<T> StrongRef<T>::GetWeak() const
{
	return m_Object;
}

inline bool ReferenceCounted::Drop() const
{
	lxAssert(m_ReferenceCounter > 0);

	if(m_ReferenceCounter == 1) {
		// Run not in destructur since Destroy may access the object itself
		auto weak = m_FirstWeak;
		while(weak) {
			auto next = weak->m_Next;
			weak->m_Next = nullptr;
			weak->Destroy();
			weak->m_Prev = nullptr;
			weak = next;
		}
		LUX_FREE(this);
		return true;
	} else {
		--m_ReferenceCounter;
	}

	return false;
}

}

#endif
