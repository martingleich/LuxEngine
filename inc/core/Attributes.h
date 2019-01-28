#ifndef INCLUDED_LUX_ATTRIBUTES_H
#define INCLUDED_LUX_ATTRIBUTES_H
#include "core/ReferenceCounted.h"
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
	virtual u32 GetChangeId() = 0;
};

class AttributeAnyImpl : public Attribute
{
public:
	AttributeAnyImpl(core::StringView name, core::Type type, const void* init = nullptr) :
		m_Any(type, init),
		m_Name(name)
	{
	}

	const core::String& GetName() const override { return m_Name; }

	core::Type GetType() const override { return m_Any.GetType(); }

	VariableAccess GetAccess(bool isConst = false) override
	{
		if(!isConst)
			m_ChangeId++;
		auto type = m_Any.GetType();
		if(isConst)
			type = type.GetConstantType();
		return VariableAccess(type, (void*)m_Any.Data());
	}

	u32 GetChangeId() override { return m_ChangeId; }

private:
	AnyObject m_Any;
	core::String m_Name;
	u32 m_ChangeId=0;
};

class AttributePtr
{
public:
	AttributePtr() { } 
	AttributePtr(Attribute* attr) { m_Weak = attr; }
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

	Attribute* operator->() const
	{
		return m_Weak;
	}

private:
	WeakRef<Attribute> m_Weak;
};

class AttributeListBuilder;
class AttributeListInternal : public ReferenceCounted
{
	friend class AttributeListBuilder;
public:
	using ConstIterator = core::HashMap<core::String, StrongRef<Attribute>>::ConstKeyIterator;
public:
	core::VariableAccess Attribute(core::StringView name)
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.end()) {
			if(m_Base)
				return m_Base->Attribute(name);
			throw core::ObjectNotFoundException(name);
		}
		return it->value->GetAccess(false);
	}

	core::VariableAccess Attribute(core::StringView name) const
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.end()) {
			if(m_Base)
				return ((const AttributeListInternal*)m_Base)->Attribute(name);
			throw core::ObjectNotFoundException(name);
		}
		return it->value->GetAccess(true);
	}

	ConstIterator First() const
	{
		return m_ObjectMap.Keys().First();
	}

	ConstIterator End() const
	{
		return m_ObjectMap.Keys().End();
	}

	AttributePtr Pointer(const core::StringView& name) const
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.end()) {
			if(m_Base)
				return m_Base->Pointer(name);
			return nullptr;
		}
		return (core::Attribute*)it->value;
	}

	AttributeListInternal* GetBase() const
	{
		return m_Base;
	}

private:
	core::HashMap<core::String, StrongRef<core::Attribute>> m_ObjectMap;
	StrongRef<AttributeListInternal> m_Base;
};

class AttributeList
{
	friend class AttributeListBuilder;
public:
	using ConstIterator = AttributeListInternal::ConstIterator;
public:
	AttributeList()
	{
	}
	AttributeList(AttributeListInternal* ptr) :
		m_Internal(ptr)
	{
	}
	core::VariableAccess operator[](core::StringView name) { return m_Internal->Attribute(name); }
	core::VariableAccess operator[](core::StringView name) const { return m_Internal->Attribute(name); }
	ConstIterator First() const { return m_Internal->First(); }
	ConstIterator End() const { return m_Internal->End(); }
	AttributePtr Pointer(const core::StringView& name) const { return m_Internal->Pointer(name); }
	AttributeList GetBase() const { return m_Internal->GetBase(); }
	bool IsValid() const { return m_Internal != nullptr; }
	bool operator==(AttributeList other) const { return m_Internal == other.m_Internal; }
	bool operator!=(AttributeList other) const { return !(*this == other); }

private:
	StrongRef<AttributeListInternal> m_Internal;
};

class AttributeListBuilder : core::Uncopyable
{
public:
	template <typename T>
	AttributePtr AddAttribute(core::StringView name, const T& value)
	{
		return AddAttribute(name, core::TemplType<T>::Get(), &value);
	}

	AttributePtr AddAttribute(Attribute* attrb)
	{
		LX_CHECK_NULL_ARG(attrb);

		auto& name = attrb->GetName();
		auto type = attrb->GetType();
		auto it = Objects().Find(name);
		if(it != Objects().end()) {
			if(it->value->GetType() != type)
				throw core::InvalidOperationException("Attribute is already defined with diffrent type");
			it->value = attrb;
		} else {
			Objects()[name] = attrb;
		}

		return attrb;
	}

	AttributePtr AddAttribute(const core::String& name, core::Type type, const void* value)
	{
		AttributePtr ptr;
		auto it = Objects().Find(name);
		if(it != Objects().end()) {
			if(it->value->GetType() != type)
				throw core::InvalidOperationException("Attribute is already defined with diffrent type");
			it->value->GetAccess().AssignData(value);
			ptr = (Attribute*)it->value;
		} else {
			auto p = Objects().At(name, LUX_NEW(AttributeAnyImpl)(name, type, value));
			Objects()[name] = p;
			ptr = (Attribute*)p;
		}

		return ptr;
	}

	void SetBase(AttributeList base)
	{
		m_Base = base.m_Internal;
	}
	
	AttributeList BuildAndReset()
	{
		m_List->m_Base = m_Base;
		auto out = m_List;
		m_List = nullptr;
		m_Base = nullptr;
		return AttributeList(out);
	}

private:
	core::HashMap<core::String, StrongRef<Attribute>>& Objects()
	{
		if(!m_List)
			m_List = LUX_NEW(AttributeListInternal)();
		return m_List->m_ObjectMap;
	}

private:
	StrongRef<AttributeListInternal> m_List;
	StrongRef<AttributeListInternal> m_Base;
};

inline AttributeList::ConstIterator begin(const AttributeList& attributes) { return attributes.First(); }
inline AttributeList::ConstIterator end(const AttributeList& attributes) { return attributes.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ATTRIBUTES_H