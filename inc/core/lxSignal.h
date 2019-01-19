#ifndef INCLUDED_LUX_LX_SIGNAL_H
#define INCLUDED_LUX_LX_SIGNAL_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include <memory>
#include <utility>

namespace lux
{
namespace core
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

template <typename Class, typename... Args>
struct SignalMemberFuncSafe : SignalFunc<Args...>
{
	using MemPtr = void (Class::*)(Args...);

	WeakRef<Class> owner;
#ifdef _MSC_VER
	// False positive bug in vc
#pragma warning(push)
#pragma warning(disable: 4121)
#endif
	MemPtr proc;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

template <typename FunctorT, typename... Args>
struct SignalFunctor : SignalFunc<Args...>
{
	mutable FunctorT proc;

	SignalFunctor(FunctorT p) :
		proc(p)
	{
	}

	void Call(Args... args) const
	{
		proc(args...);
	}

	bool Equal(const SignalFunc<Args...>&) const
	{
		// Can't compare functors
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
class SingleSignal
{
	template <typename Class, typename... Args2>
	using SignalMemberFuncAuto = typename core::Choose<
		std::is_base_of<ReferenceCounted, Class>::value,
		SignalMemberFuncSafe<Class, Args2...>,
		SignalMemberFuncUnsafe<Class, Args2...>>::type;
public:
	SingleSignal() {}
	SingleSignal(const SingleSignal&)
	{
	}
	SingleSignal(SingleSignal&& old) :
		m_Event(std::move(old.m_Event))
	{
	}

	SingleSignal& operator=(const SingleSignal&)
	{
		m_Event = nullptr;
		return *this;
	}
	SingleSignal& operator=(SingleSignal&& old)
	{
		m_Event = std::move(old.m_Event);
		return *this;
	}

	template <typename Class>
	void Connect(StrongRef<Class>owner, void (Class::*proc)(Args...))
	{
		Connect((Class*)owner, proc);
	}

	template <typename Class>
	void Connect(WeakRef<Class> owner, void (Class::*proc)(Args...))
	{
		Connect((Class*)owner, proc);
	}

	template <typename Class>
	void Connect(Class* owner, void (Class::*proc)(Args...))
	{
		if(owner && proc)
			m_Event = std::unique_ptr<SignalMemberFuncAuto<Class, Args...>>(
				new SignalMemberFuncAuto<Class, Args...>(owner, proc));
	}

	void Connect(void(*proc)(Args...))
	{
		if(proc)
			m_Event = std::unique_ptr<SignalStaticFunc<Args...>>(
				new SignalStaticFunc<Args...>(proc));
	}

	template <typename FunctorT>
	void Connect(const FunctorT& functor)
	{
		m_Event = std::unique_ptr<SignalFunctor<FunctorT, Args...>>(
			new SignalFunctor<FunctorT, Args...>(functor));
	}

	void Disconnect()
	{
		m_Event = nullptr;
	}

	void Call(Args... args) const
	{
		m_Event->Call(args...);
	}

	bool IsBound() const
	{
		return m_Event != nullptr;
	}

	const SignalFunc<Args...>* GetFunc() const { return m_Event.get(); }
	SignalFunc<Args...>* GetFunc() { return m_Event.get(); }
private:
	std::unique_ptr<SignalFunc<Args...>> m_Event;
};

template <typename... Args>
class Signal
{
	friend class SignalRef<Args...>;

	template <typename Class, typename... Args2>
	using SignalMemberFuncAuto = typename core::Choose<
		std::is_base_of<ReferenceCounted, Class>::value,
		SignalMemberFuncSafe<Class, Args2...>,
		SignalMemberFuncUnsafe<Class, Args2...>>::type;

public:
	Signal() :
		m_FirstRef(nullptr)
	{
	}

	~Signal();

	// Signals can be copied but will lose all their listeners on the way.
	Signal(const Signal&) :
		m_FirstRef(nullptr)
	{
	}

	// Signals can be copied but will lose all their listeners on the way.
	Signal& operator=(const Signal&)
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

	template <typename... Args2>
	void Connect(Args2&&... args)
	{
		SingleSignal<Args...> signal;
		signal.Connect(std::forward<Args2>(args)...);
		m_Callfuncs.PushBack(std::move(signal));
		if(m_ConnectEvent.IsBound())
			m_ConnectEvent.Call(*m_Callfuncs.Back().GetFunc());
	}

	template <typename Class>
	void Disconnect(Class* owner, void (Class::*proc)(Args...))
	{
		SignalMemberFuncAuto<Class, Args...> compare_dummy(owner, proc);
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End(); ++it) {
			if(it->GetFunc()->Equal(compare_dummy)) {
				it = m_Callfuncs.Erase(it);
				break;
			}
		}
	}

	void Disconnect(void(*proc)(Args...))
	{
		SignalStaticFunc<Args...> compare_dummy(proc);
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End(); ++it) {
			if(it->GetFunc()->Equal(compare_dummy)) {
				it = m_Callfuncs.Erase(it);
				break;
			}
		}
	}

	void DisconnectClass(const void* owner)
	{
		for(int i = 0; i < m_Callfuncs.Size();) {
			auto& func = m_Callfuncs[i];
			if(func.GetFunc()->GetOwner() == owner)
				m_Callfuncs.Erase(i);
			else
				++i;
		}
	}

	void DisconnectAll()
	{
		m_Callfuncs.Clear();
	}

	template <typename... Args2>
	void Broadcast(Args2&&... args) const
	{
		for(int i = 0; i < m_Callfuncs.Size();) {
			auto& func = m_Callfuncs[i];
			if(func.GetFunc()->IsDestroyed()) {
				m_Callfuncs.Erase(i);
			} else {
				func.Call(std::forward<Args2>(args)...);
				++i;
			}
		}
	}

	bool IsBound() const
	{
		return !m_Callfuncs.IsEmpty();
	}

	template <typename... ConnectArgs>
	void SetConnectEvent(ConnectArgs&&... args)
	{
		m_ConnectEvent.Connect(std::forward<ConnectArgs>(args)...);
	}

private:
	SignalRef<Args...>* m_FirstRef;
	SingleSignal<const SignalFunc<Args...>&> m_ConnectEvent;
	mutable core::Array<SingleSignal<Args...>> m_Callfuncs;
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

#endif // #ifndef INCLUDED_LUX_LX_SIGNAL_H
