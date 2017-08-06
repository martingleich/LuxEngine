#ifndef INCLUDED_ATTRIBUTES_H
#define INCLUDED_ATTRIBUTES_H
#include "core/ParamPackage.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace core
{

class Attributes
{
public:
	template <typename T>
	void AddAttribute(const String& name, const T& value)
	{
		AnyObject obj(core::GetTypeInfo<T>());
		obj.Assign(&value);
		m_ObjectMap[name] = obj;
	}

	void RemoveAttribute(const String& name)
	{
		m_ObjectMap.Erase(name);
	}

	core::VariableAccess operator[](const String& name)
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.End())
			throw core::ObjectNotFoundException(name.Data_c());
		return core::VariableAccess(it.value().GetType(), it.key().Data(), it.value().Data());
	}

	core::VariableAccess operator[](const String& name) const
	{
		auto it = m_ObjectMap.Find(name);
		if(it == m_ObjectMap.End())
			throw core::ObjectNotFoundException(name.Data_c());
		return core::VariableAccess(it.value().GetType().GetConstantType(), it.key().Data(), const_cast<void*>(it.value().Data()));
	}

	core::HashMap<String, AnyObject>::ConstKeyIterator First() const
	{
		return m_ObjectMap.FirstKeyC();
	}

	core::HashMap<String, AnyObject>::ConstKeyIterator End() const
	{
		return m_ObjectMap.EndKeyC();
	}

	core::HashMap<String, AnyObject>::ConstKeyIterator begin() const
	{
		return First();
	}

	core::HashMap<String, AnyObject>::ConstKeyIterator end() const
	{
		return End();
	}

private:
	core::HashMap<String, AnyObject> m_ObjectMap;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_ATTRIBUTES_H