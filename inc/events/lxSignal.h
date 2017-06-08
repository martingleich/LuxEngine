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
struct signal_func
{
	virtual ~signal_func() {}
	virtual void Call(Args...) const = 0;
	virtual bool Equal(const signal_func& other) const = 0;
	virtual const void* GetOwner() const = 0;
	virtual bool IsDestroyed() const = 0;
};

namespace impl
{
template <typename T1, typename T2, bool First>
struct Select;

template <typename T1, typename T2>
struct Select<T1, T2, true> { using type = T1; };

template <typename T1, typename T2>
struct Select<T1, T2, false> { using type = T2; };
}

template <typename Class, typename... Args>
struct signal_memberfunc_safe : signal_func<Args...>
{
	using MemPtr = typename void (Class::*)(Args...);

	WeakRef<Class> owner;
	MemPtr proc;

	signal_memberfunc_safe(Class* o, MemPtr proc) :
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

	bool Equal(const signal_func<Args...>& other) const
	{
		auto that = dynamic_cast<const signal_memberfunc_safe*>(&other);
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
struct signal_memberfunc_unsafe : signal_func<Args...>
{
	using MemPtr = typename void (Class::*)(Args...);

	Class* owner;
	MemPtr proc;

	signal_memberfunc_unsafe(Class* o, MemPtr proc) :
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

	bool Equal(const signal_func<Args...>& other) const
	{
		auto that = dynamic_cast<const signal_memberfunc_unsafe*>(&other);
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
struct signal_staticfunc : signal_func<Args...>
{
	using FuncPtr = typename void(*)(Args...);

	FuncPtr proc;

	signal_staticfunc(FuncPtr p) :
		proc(p)
	{
		lxAssert(proc);
	}

	void Call(Args... args) const
	{
		proc(args...);
	}

	bool Equal(const signal_func<Args...>& other) const
	{
		auto that = dynamic_cast<const signal_staticfunc*>(&other);
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
class signal_ref;

template <typename... Args>
class signal
{
	friend class signal_ref<Args...>;

	template <typename Class, typename... Args>
	using signal_memberfunc_auto = typename impl::Select<
		signal_memberfunc_safe<Class, Args...>,
		signal_memberfunc_unsafe<Class, Args...>,
		std::is_base_of<ReferenceCounted, Class>::value>::type;

public:
	signal() :
		m_FirstRef(nullptr)
	{
	}

	~signal();

	// Signals can be copied but will lose all their listeners on the way.
	signal(const signal& other) :
		m_FirstRef(nullptr)
	{
	}

	// Signals can be copied but will lose all their listeners on the way.
	signal& operator=(const signal& other)
	{
		~signal();
		m_FirstRef = nullptr;
		return *this;
	}

	signal(signal&& old) :
		m_FirstRef(nullptr)
	{
		*this = std::move(old);
	}

	signal& operator=(signal&& old);

	template <typename Class>
	void Connect(Class* owner, void (Class::*proc)(Args...))
	{
		if(owner && proc)
			m_Callfuncs.PushBack(std::make_unique<signal_memberfunc_auto<Class, Args...>>(owner, proc));
	}

	void Connect(void(*proc)(Args...))
	{
		if(proc)
			m_Callfuncs.PushBack(std::make_unique<signal_staticfunc<Args...>>(proc));
	}

	template <typename Class>
	void Disconnect(Class* owner, void (Class::*proc)(Args...))
	{
		signal_memberfunc_auto<Class, Args...> compare_dummy(owner, proc);
		for(auto it = m_Callfuncs.First(); it != m_Callfuncs.End(); ++it) {
			if((*it)->Equal(compare_dummy)) {
				it = m_Callfuncs.Erase(it);
				break;
			}
		}
	}

	void Disconnect(void(*proc)(Args...))
	{
		signal_staticfunc<Args...> compare_dummy(proc);
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

	template <typename... Args>
	void Broadcast(Args... args) const
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
	signal_ref<Args...>* m_FirstRef;
	mutable core::array<std::unique_ptr<signal_func<Args...>>> m_Callfuncs;
};

template <typename... Args>
class signal_ref
{
	friend class signal<Args...>;
public:
	signal_ref(signal<Args...>& s) :
		signal_ref(&s)
	{

	}

	signal_ref(signal<Args...>* s)
	{
		signal = s;

		if(signal) {
			this->prev = nullptr;
			this->next = signal->m_FirstRef;
			if(signal->m_FirstRef)
				signal->m_FirstRef->prev = this;
			signal->m_FirstRef = this;
		} else {
			this->next = this->prev = nullptr;
		}
	}

	signal_ref(const signal_ref& other) : signal_ref(other.signal)
	{
	}

	~signal_ref()
	{
		if(signal) {
			if(next)
				next->prev = prev;
			if(prev)
				prev->next = next;
			if(this == signal->m_FirstRef)
				signal->m_FirstRef = next;
		}
	}

	signal_ref& operator=(const signal_ref& other)
	{
		this->~signal_ref();
		signal = other.signal;

		if(signal) {
			this->next = signal->m_FirstRef;
			if(signal->m_FirstRef)
				signal->m_FirstRef->prev = this;
			signal->m_FirstRef = this;
		} else {
			next = prev = nullptr;
		}

		return *this;
	}

	signal<Args...>* operator->() const
	{
		return signal;
	}

	bool operator==(const signal<Args...>& s) const
	{
		return signal == &s;
	}

	bool operator==(const signal<Args...>* s) const
	{
		return signal == s;
	}

	bool operator==(const signal_ref& other) const
	{
		return signal == other.signal;
	}

	bool operator!=(const signal<Args...>& s) const
	{
		return signal != &s;
	}

	bool operator!=(const signal<Args...>* s) const
	{
		return signal != s;
	}

	bool operator!=(const signal_ref& other) const
	{
		return signal != other.signal;
	}

	operator signal<Args...>*() const
	{
		return signal;
	}

	void ChangeSignal(signal<Args...>* newPtr)
	{
		signal_ref* ptr = this;
		while(ptr) {
			ptr->signal = newPtr;
			ptr = ptr->next;
		}
	}

private:
	signal<Args...>* signal;
	signal_ref* next;
	signal_ref* prev;
};

template <typename... Args>
inline signal<Args...>::~signal()
{
	auto* cur = m_FirstRef;
	while(cur) {
		auto tmp = m_FirstRef->next;
		cur->signal = nullptr;
		cur->next = nullptr;
		cur->prev = nullptr;
		cur = tmp;
	}

	DisconnectAll();
}

template <typename... Args>
signal<Args...>& signal<Args...>::operator=(signal&& old)
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