#include "core/lxID.h"
#include "core/lxArray.h"

namespace lux
{
namespace core
{

struct IDManager::IDList
{
	core::Array<core::Referable*> objects;
	u32 biggest = 1;
	core::Array<u32> recycled;

	u32 Get(core::Referable* object)
	{
		u32 value;
		if(!recycled.IsEmpty()) {
			value = recycled.Back();
			recycled.PopBack();
		} else {
			value = biggest;
			++biggest;
		}
		if((u32)objects.Size() < value)
			objects.Resize(objects.Size() * 2 + 1);
		objects[value - 1] = object;
		return value;
	}

	void Release(u32 id)
	{
		recycled.PushBack(id);
	}

	core::Referable* LookUp(u32 id)
	{
		if(id == 0)
			return nullptr;
		id--;
		if(id >= (u32)objects.Size())
			return nullptr;
		return objects[id];
	}
};

static StrongRef<IDManager> g_Manager;
IDManager* IDManager::Instance()
{
	if(!g_Manager)
		g_Manager = LUX_NEW(IDManager);
	return g_Manager;
}

IDManager::IDManager() :
	m_List(std::make_unique<IDList>())
{
}

IDManager::~IDManager()
{
}

ID IDManager::Create(core::Referable* obj)
{
	u32 value = m_List->Get(obj);
	return ID(value);
}

void IDManager::Release(ID id)
{
	u32 value = id.GetValue();
	m_List->Release(value);
}

core::Referable* IDManager::LookUp(ID id)
{
	return m_List->LookUp(id.GetValue());
}

namespace Types
{

Type StrongID()
{
	static Type strongRef(LUX_NEW(TypeInfoTemplate<core::ID>)("strong_id"));
	return strongRef;
}

Type WeakID()
{
	static Type weakRef(LUX_NEW(TypeInfoTemplate<core::ID>)("weak_id"));
	return weakRef;
}

bool IsIDType(Type t)
{
	return t == StrongID() || t == WeakID();
}

} // namespace Types
} // namespace core
} // namespace lux