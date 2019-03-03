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
	Attribute(core::StringView name, const core::Type& type) :
		m_Name(name),
		m_Type(type),
		m_ChangeId(0)
	{
	}

	const core::String& GetName() const { return m_Name; }
	core::Type GetType() const { return m_Type; }
	u32 GetChangeId() const { return m_ChangeId; }

	template <typename T>
	const T& GetValue()
	{
		// TODO: Check type
		return *(const T*)GetValuePtr();
	}
	const void* GetValuePointer()
	{
		return GetValuePtr();
	}

	template <typename T>
	void SetValue(const T& value)
	{
		// TODO: Check type
		SetValuePtr(&value);
	}

protected:
	virtual void SetValuePtr(const void* ptr) = 0;
	virtual const void* GetValuePtr() = 0;

protected:
	core::String m_Name;
	core::Type m_Type;
	u32 m_ChangeId;
};

template <typename T>
class AttributeAnyImpl : public Attribute
{
public:
	AttributeAnyImpl(core::StringView name, core::Type type, const T& init) :
		Attribute(name, type),
		m_Value(init)
	{
	}

	void SetValuePtr(const void* ptr) override
	{
		m_ChangeId++;
		VariableAccess(m_Type, &m_Value).AssignData(ptr);
	}
	const void* GetValuePtr() override
	{
		return &m_Value;
	}

private:
	T m_Value;
};

class AttributePtr
{
public:
	AttributePtr() { } 
	explicit AttributePtr(Attribute* attr) { m_Weak = attr; }
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
	ConstIterator First() const { return m_ObjectMap.Keys().First(); }
	ConstIterator End() const { return m_ObjectMap.Keys().End(); }

	AttributePtr Pointer(const core::StringView& name) const
	{
		auto itOpt = m_ObjectMap.Find(name);
		if(!itOpt.HasValue()) {
			if(m_Base)
				return m_Base->Pointer(name);
			return AttributePtr(nullptr);
		}
		return AttributePtr(itOpt.GetValue()->value);
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
	AttributeList() { }
	AttributeList(AttributeListInternal* ptr) :
		m_Internal(ptr)
	{
	}

	template <typename T>
	const T& GetValue(core::StringView name)
	{
		return m_Internal->Pointer(name)->GetValue<T>();
	}
	template <typename T>
	void SetValue(core::StringView name, const T& value)
	{
		m_Internal->Pointer(name)->SetValue(value);
	}

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
		AttributePtr ptr;
		auto type = core::TemplType<T>::Get();
		auto itOpt = Objects().Find(name);
		if(itOpt.HasValue()) {
			if(itOpt.GetValue()->value->GetType() != type)
				throw core::InvalidOperationException("Attribute is already defined with diffrent type");
			itOpt.GetValue()->value->SetValue(value);
			ptr = AttributePtr(itOpt.GetValue()->value);
		} else {
			StrongRef<Attribute> p = LUX_NEW(AttributeAnyImpl<T>)(name, type, value);
			Objects().SetAndReplace(name, p);
			Objects()[name] = p;
			ptr = AttributePtr(p);
		}

		return ptr;
	}

	AttributePtr AddAttribute(Attribute* attrb)
	{
		LX_CHECK_NULL_ARG(attrb);

		auto& name = attrb->GetName();
		auto type = attrb->GetType();
		auto itOpt = Objects().Find(name);
		if(itOpt.HasValue()) {
			if(itOpt.GetValue()->value->GetType() != type)
				throw core::InvalidOperationException("Attribute is already defined with diffrent type");
			itOpt.GetValue()->value = attrb;
		} else {
			Objects()[name] = attrb;
		}

		return AttributePtr(attrb);
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