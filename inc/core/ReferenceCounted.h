#ifndef INCLUDED_REFERENCECOUNTED_H
#define INCLUDED_REFERENCECOUNTED_H
#include "LuxBase.h"
#include "lxException.h"
#include <cstddef>
#include <vector>

namespace lux
{

class ReferenceCounted;

class WeakRefBase
{
	friend class ReferenceCounted;
public:
	WeakRefBase() :
		m_Next(nullptr),
		m_Prev(nullptr)
	{
	}
	WeakRefBase(WeakRefBase* n, WeakRefBase* p) :
		m_Next(n),
		m_Prev(p)
	{
	}

	virtual ~WeakRefBase()
	{
	}

protected:
	virtual void Destroy() = 0;

	void AddTo(ReferenceCounted* obj);
	void AssignTo(ReferenceCounted* from, ReferenceCounted* to);
	void RemoveFrom(ReferenceCounted* obj);

public:
	WeakRefBase* m_Next;
	WeakRefBase* m_Prev;
};

//! A object implementing reference counting
class ReferenceCounted
{
	friend class WeakRefBase;

private:
	mutable int m_ReferenceCounter;
	WeakRefBase* m_FirstWeak;

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

inline void WeakRefBase::AddTo(ReferenceCounted* obj)
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

inline void WeakRefBase::AssignTo(ReferenceCounted* from, ReferenceCounted* to)
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

inline void WeakRefBase::RemoveFrom(ReferenceCounted* obj)
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
	StrongRef() : m_Object(nullptr)
	{
	}

	StrongRef(T* obj) : m_Object(obj)
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
	StrongRef(const StrongRef<T2>& other) : m_Object(static_cast<T*>(other.m_Object))
	{
		if(m_Object)
			m_Object->Grab();
	}

	StrongRef(const WeakRef<T>& other);

	StrongRef<T>& operator=(const StrongRef<T>& other)
	{
		*this = other.m_Object;
		return *this;
	}

	template <typename T2>
	StrongRef<T>& operator=(const StrongRef<T2>& other)
	{
		*this = static_cast<T*>(other.m_Object);
		return *this;
	}

	template <typename T2>
	StrongRef<T>& operator=(StrongRef<T2>&& old)
	{
		if(m_Object)
			m_Object->Drop();

		m_Object = static_cast<T*>(old.m_Object);
		old.m_Object = nullptr;

		return *this;
	}

	~StrongRef()
	{
		if(m_Object)
			m_Object->Drop();
		m_Object = nullptr;
	}

	template <typename T2>
	StrongRef<T>& operator=(T2* obj)
	{
		if(m_Object)
			m_Object->Drop();

		if(obj)
			obj->Grab();

		m_Object = obj;
		return *this;
	}

	StrongRef<T>& operator=(std::nullptr_t)
	{
		if(m_Object)
			m_Object->Drop();
		m_Object = nullptr;
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

	T* operator*() const
	{
		return m_Object;
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
	operator StrongRef<T2>() const
	{
		T2* cast_out = dynamic_cast<T2*>(m_Object);
		if(!cast_out)
			throw core::InvalidCastException();
		return StrongRef<T2>(cast_out);
	}

	template <typename T2>
	T2* As() const
	{
		return dynamic_cast<T2*>(m_Object);
	}
};

template <typename T>
class WeakRef : public WeakRefBase
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

	WeakRef(const WeakRef<T>& other) : WeakRef(other.m_Object)
	{
	}

	WeakRef(const StrongRef<T>& other) : WeakRef(*other)
	{
	}

	WeakRef& operator=(const WeakRef& other)
	{
		return this->operator=(dynamic_cast<T*>(other.m_Object));
	}

	template <typename T2>
	WeakRef(const WeakRef<T2>& other) : WeakRef(static_cast<T2*>(other.m_Object))
	{
	}

	template <typename T2>
	WeakRef(const StrongRef<T2>& other) : WeakRef(static_cast<T2*>(*other))
	{

	}

	template <typename T2>
	WeakRef<T>& operator=(const WeakRef<T2>& other)
	{
		auto cast_out = static_cast<T*>(other.m_Object);
		return this->operator=(cast_out);
	}

	~WeakRef()
	{
		RemoveFrom(m_Object);
		m_Object = nullptr;
	}

	void Destroy()
	{
		m_Object = nullptr;
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

	T* operator*() const
	{
		return m_Object;
	}

	StrongRef<T> GetStrong()
	{
		return StrongRef<T>(m_Object);
	}

	void Reset()
	{
		this->operator=(nullptr);
	}

	bool operator!() const
	{
		return !m_Object;
	}

	template <typename T2>
	T2* As() const
	{
		return dynamic_cast<T2*>(m_Object);
	}
};

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
	return WeakRef<T>(m_Object);
}

template <typename T>
StrongRef<T>::StrongRef(const WeakRef<T>& other) : StrongRef(*other)
{

}

inline  bool ReferenceCounted::Drop() const
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

inline ReferenceCounted::~ReferenceCounted()
{
}

}

#endif
