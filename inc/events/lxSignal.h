#ifndef INCLUDED_LX_SIGNAL_H
#define INCLUDED_LX_SIGNAL_H
#include "core/lxArray.h"
#include "core/ReferenceCounted.h"
#include <memory>
#include <utility>

namespace lux
{
namespace events
{
template <typename... Args>
struct SignalFunc
{
	virtual ~SignalFunc() {}
	virtual void Call(Args...) const = 0;
	virtual bool Equal(const SignalFunc& other) const = 0;
	virtual const void* GetOwner() const = 0;
	virtual bool IsDestroyed() const = 0;
};

namespace signal_impl
{
template <typename T1, typename T2, bool First>
struct Select;

template <typename T1, typename T2>
struct Select<T1, T2, true> { using type = T1; };

template <typename T1, typename T2>
struct Select<T1, T2, false> { using type = T2; };
}

template <typename Class, typename... Args>
struct SignalMemberFuncSafe : SignalFunc<Args...>
{
	using MemPtr = void (Class::*)(Args...);

	WeakRef<Class> owner;
	MemPtr proc;

	SignalMemberFuncSafe(Class* o, MemPtr proc) :
		owner(o),
		proc(proc)
	{
		lxAssert(owner);
		lxAssert(proc);
	}

	void Call(Args... args) const
	{
		(owner->*proc)(args...);
	}

	bool Equal(const SignalFunc<Args...>& other) const
	{
		auto that = dynamic_cast<const SignalMemberFuncSafe*>(&other);
		if(!that)
			return false;

		if((that->owner == nullptr || that->owner == this->owner) && (that->proc == nullptr || that->proc == this->proc))
			return true;
		else
			return false;
	}

	const void* GetOwner() const
	{
		return owner;
	}

	bool IsDestroyed() const
	{
		return (owner == nullptr);
	}
};

template <typename Class, typename... Args>
struct SignalMemberFuncUnsafe : SignalFunc<Args...>
{
	using MemPtr = void (Class::*)(Args...);

	Class* owner;
	MemPtr proc;

	SignalMemberFuncUnsafe(Class* o, MemPtr proc) :
		owner(o),
		proc(proc)
	{
		lxAssert(owner);
		lxAssert(proc);
	}

	void Call(Args... args) const
	{
		(owner->*proc)(args...);
	}

	bool Equal(const SignalFunc<Args...>& other) const
	{
		auto that = dynamic_cast<const SignalMemberFuncUnsafe*>(&other);
		if(!that)
			return false;

		if((that->owner == nullptr || that->owner == this->owner) && (that->proc == nullptr || that->proc == this->proc))
			return true;
		else
			return false;
	}

	const void* GetOwner() const
	{
		return owner;
	}

	bool IsDestroyed() const
	{
		return false;
	}
};

template <typename... Args>
struct SignalStaticFunc : SignalFunc<Args...>
{
	using FuncPtr = void(*)(Args...);

	FuncPtr proc;

	SignalStaticFunc(FuncPtr p) :
		proc(p)
	{
		lxAssert(proc);
	}

	void Call(Args... args) const
	{
		proc(args...);
	}

	bool Equal(const SignalFunc<Args...>& other) const
	{
		auto that = dynamic_cast<const SignalStaticFunc*>(&other);
		if(!that)
			return false;

		if(that->proc == nullptr || that->proc == this->proc)
			return true;
		else
			return false;
	}

	const void* GetOwner() const
	{
		return nullptr;
	}

	bool IsDestroyed() const
	{
		return false;
	}
};

template <typename... Args>
class SignalRef;

template <typename... Args>
class Signal
{
	friend class SignalRef<Args...>;

	template <typename Class, typename... Args2>
	using SignalMemberFuncAuto = typename signal_impl::Select<
		SignalMemberFuncSafe<Class, Args2...>,
		SignalMemberFuncUnsafe<Class, Args2...>,
		std::is_base_of<ReferenceCounted, Class>::value>::type;

public:
	Signal() :
		m_FirstRef(nullptr)
	{
	}

	~Signal();

	// Signals can be copied but will lose all their listeners on the way.
	Signal(const Signal& other) :
		m_FirstRef(nullptr)
	{
	}

	// Signals can be copied but will lose all their listeners on the way.
	Signal& operator=(const Signal& other)
	{
		~Signal();
		m_FirstRef = nullptr;
		return *this;
	}

	Signal(Signal&& old) :
		m_FirstRef(nullptr)
	{
		*this = std::move(old);
	}

	Signal& operator=(Signal&& old);

	template <typename Class>
	void Connect(Class* owner, void (Class::*proc)(Args...))
	{
		if(owner && proc)
			m_Callfuncs.PushBack(std::unique_ptr<SignalMemberFuncAuto<Class, Args...>>(
				new SignalMemberFuncAuto<Class, Args...>(owner, proc)));
	}

	void Connect(void(*proc)(Args...))
	{
		if(proc)
			m_Callfuncs.PushBack(std::unique_ptr<SignalStaticFunc<Args...>>(
				new SignalStaticFunc<Args...>(proc)));
	}

	template <typename Class>
	void Disconnect(Class* owner, void (Class::*proc)(Args...))
	{
		SignalMemberFuncAuto<Class, Args...> compare_dummy(owner, proc);
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End(); ++it) {
			if((*it)->Equal(compare_dummy)) {
				it = m_Callfuncs.Erase(it);
				break;
			}
		}
	}

	void Disconnect(void(*proc)(Args...))
	{
		SignalStaticFunc<Args...> compare_dummy(proc);
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End(); ++it) {
			if((*it)->Equal(compare_dummy)) {
				it = m_Callfuncs.Erase(it);
				break;
			}
		}
	}

	void DisconnectClass(const void* owner)
	{
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End();) {
			if((*it)->GetOwner() == owner)
				it = m_Callfuncs.Erase(it, true);
			else
				++it;
		}
	}

	void DisconnectAll()
	{
		m_Callfuncs.Clear();
	}

	template <typename... Args2>
	void Broadcast(Args2... args) const
	{
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End();) {
			if((*it)->IsDestroyed()) {
				it = m_Callfuncs.Erase(it, true);
				continue;
			}

			(*it)->Call(args...);
			++it;
		}
	}

	bool IsBound() const
	{
		return !m_Callfuncs.IsEmpty();
	}

private:
	SignalRef<Args...>* m_FirstRef;
	mutable core::array<std::unique_ptr<SignalFunc<Args...>>> m_Callfuncs;
};

template <typename... Args>
class SignalRef
{
	friend class Signal<Args...>;
public:
	SignalRef(Signal<Args...>& s) :
		SignalRef(&s)
	{

	}

	SignalRef(Signal<Args...>* s)
	{
		m_Signal = s;

		if(m_Signal) {
			this->m_Prev = nullptr;
			this->m_Next = m_Signal->m_FirstRef;
			if(m_Signal->m_FirstRef)
				m_Signal->m_FirstRef->m_Prev = this;
			m_Signal->m_FirstRef = this;
		} else {
			this->m_Next = this->m_Prev = nullptr;
		}
	}

	SignalRef(const SignalRef& other) : SignalRef(other.m_Signal)
	{
	}

	~SignalRef()
	{
		if(m_Signal) {
			if(m_Next)
				m_Next->m_Prev = m_Prev;
			if(m_Prev)
				m_Prev->m_Next = m_Next;
			if(this == m_Signal->m_FirstRef)
				m_Signal->m_FirstRef = m_Next;
		}
	}

	SignalRef& operator=(const SignalRef& other)
	{
		this->~SignalRef();
		m_Signal = other.m_Signal;

		if(m_Signal) {
			this->m_Next = m_Signal->m_FirstRef;
			if(m_Signal->m_FirstRef)
				m_Signal->m_FirstRef->m_Prev = this;
			m_Signal->m_FirstRef = this;
		} else {
			m_Next = m_Prev = nullptr;
		}

		return *this;
	}

	Signal<Args...>* operator->() const
	{
		return m_Signal;
	}

	bool operator==(const Signal<Args...>& s) const
	{
		return m_Signal == &s;
	}

	bool operator==(const Signal<Args...>* s) const
	{
		return m_Signal == s;
	}

	bool operator==(const SignalRef& other) const
	{
		return m_Signal == other.m_Signal;
	}

	bool operator!=(const Signal<Args...>& s) const
	{
		return m_Signal != &s;
	}

	bool operator!=(const Signal<Args...>* s) const
	{
		return m_Signal != s;
	}

	bool operator!=(const SignalRef& other) const
	{
		return m_Signal != other.m_Signal;
	}

	operator Signal<Args...>*() const
	{
		return m_Signal;
	}

	void ChangeSignal(Signal<Args...>* newPtr)
	{
		SignalRef* ptr = this;
		while(ptr) {
			ptr->m_Signal = newPtr;
			ptr = ptr->m_Next;
		}
	}

private:
	Signal<Args...>* m_Signal;
	SignalRef* m_Next;
	SignalRef* m_Prev;
};

template <typename... Args>
inline Signal<Args...>::~Signal()
{
	auto* cur = m_FirstRef;
	while(cur) {
		auto tmp = m_FirstRef->m_Next;
		cur->m_Signal = nullptr;
		cur->m_Next = nullptr;
		cur->m_Prev = nullptr;
		cur = tmp;
	}

	DisconnectAll();
}

template <typename... Args>
Signal<Args...>& Signal<Args...>::operator=(Signal&& old)
{
	m_FirstRef = old.m_FirstRef;
	if(m_FirstRef)
		m_FirstRef->ChangeSignal(this);

	m_Callfuncs = std::move(old.m_Callfuncs);

	old.m_FirstRef = nullptr;
	old.m_Callfuncs.Clear();

	return *this;
}

}
}

#endif // #ifndef INCLUDED_LX_SIGNAL_H
