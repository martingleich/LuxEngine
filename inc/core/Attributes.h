#ifndef INCLUDED_ATTRIBUTES_H
#define INCLUDED_ATTRIBUTES_H
#include "core/ParamPackage.h"
#include "core/lxHashMap.h"
#include "core/lxOrderedMap.h"

namespace lux
{
namespace core
{

class Attribute : public ReferenceCounted
{
public:
	virtual const core::String& GetName() const = 0;
	virtual core::Type GetType() const = 0;
	virtual VariableAccess GetAccess(bool isConst = false) = 0;
};

class AttributeAnyImpl : public Attribute
{
public:
	AttributeAnyImpl(const core::String& name, core::Type type, const void* init = nullptr) :
		m_Any(type, init),
		m_Name(name)
	{
	}

	virtual const core::String& GetName() const
	{
		return m_Name;
	}

	virtual core::Type GetType() const
	{
		return m_Any.GetType();
	}

	virtual VariableAccess GetAccess(bool isConst = false)
	{
		return VariableAccess(m_Any, isConst);
	}

private:
	AnyObject m_Any;
	core::String m_Name;
};

class AttributePtr
{
public:
	AttributePtr()
	{
	}

	AttributePtr(Attribute* attr)
	{
		m_Weak = attr;
	}

	AttributePtr(const AttributePtr& other) :
		m_Weak(other.m_Weak)
	{
	}

	AttributePtr(AttributePtr&& old) :
		m_Weak(std::move(old.m_Weak))
	{
	}

	AttributePtr& operator=(const AttributePtr& other)
	{
		m_Weak = other.m_Weak;
		return *this;
	}

	AttributePtr& operator=(AttributePtr&& old)
	{
		m_Weak = std::move(old.m_Weak);
		return *this;
	}

	bool operator==(const AttributePtr& other) const
	{
		return m_Weak == other.m_Weak;
	}

	bool operator!=(const AttributePtr& other) const
	{
		return !(*this == other);
	}

	bool operator!() const
	{
		return (m_Weak == nullptr);
	}

	operator bool() const
	{
		return (m_Weak != nullptr);
	}

	VariableAccess operator*() const
	{
		return m_Weak->GetAccess();
	}

	Attribute* operator->() const
	{
		return m_Weak;
	}

private:
	WeakRef<Attribute> m_Weak;
};

class Attributes
{
public:
	using ConstIterator = core::HashMap<core::String, StrongRef<Attribute>>::ConstKeyIterator;
public:
	template <typename T>
	AttributePtr AddAttribute(const core::String& name, const T& value)
	{
		return AddAttribute(name, core::TemplType<T>::Get(), &value);
	}

	AttributePtr AddAttribute(Attribute* attrb)
	{
		LX_CHECK_NULL_ARG(attrb);

		auto name = attrb->GetName();
		auto type = attrb->GetType();
		auto it = m_ObjectMap.Find(name);
		if(it != m_ObjectMap.End()) {
			if((*it)->GetType() != type)
				throw core::Exception("Attribute is already defined with diffrent type");
			*it = attrb;
		} else {
			m_ObjectMap[name] = attrb;
		}

		return attrb;
	}

	AttributePtr AddAttribute(const core::String& name, core::Type type, const void* value)
	{
		AttributePtr ptr;
		auto it = m_ObjectMap.Find(name);
		if(it != m_ObjectMap.End()) {
			if((*it)->GetType() != type)
				throw core::Exception("Attribute is already defined with diffrent type");
			(*it)->GetAccess().AssignData(value);
			ptr = (Attribute*)(*it);
		} else {
			auto p = m_ObjectMap.At(name, LUX_NEW(AttributeAnyImpl)(name, type, value));
			m_ObjectMap[name] = p;
			ptr = (Attribute*)p;
		}

		return ptr;
	}

	void RemoveAttribute(const core::String& name)
	{
		m_ObjectMap.Erase(name);
	}

	core::VariableAccess operator[](const core::String& name)
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.End())
			throw core::ObjectNotFoundException(name.Data_c());
		return (*it)->GetAccess(false);
	}

	core::VariableAccess operator[](const core::String& name) const
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.End())
			throw core::ObjectNotFoundException(name.Data_c());
		return (*it)->GetAccess(true);
	}

	ConstIterator First() const
	{
		return m_ObjectMap.FirstKey();
	}

	ConstIterator End() const
	{
		return m_ObjectMap.EndKey();
	}

	AttributePtr Pointer(const core::String& name) const
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.End())
			return nullptr;
		return (Attribute*)(*it);
	}

private:
	core::HashMap<core::String, StrongRef<Attribute>> m_ObjectMap;
};

inline Attributes::ConstIterator begin(const Attributes& attributes) { return attributes.First(); }
inline Attributes::ConstIterator end(const Attributes& attributes) { return attributes.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_ATTRIBUTES_H