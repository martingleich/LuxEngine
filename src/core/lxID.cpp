#include "core/lxID.h"
#include "core/lxArray.h"

namespace lux
{
namespace core
{

struct IDManager::IDList
{
	core::Array<Referable*> objects;
	u32 biggest = 1;
	core::Array<u32> recycled;

	u32 Get(Referable* object)
	{
		u32 value;
		if(!recycled.IsEmpty()) {
			value = recycled.Back();
			recycled.PopBack();
		} else {
			value = biggest;
			++biggest;
		}
		if(objects.Size() < value)
			objects.Resize(objects.Size() * 2 + 1);
		objects[value - 1] = object;
		return value;
	}

	void Release(u32 id)
	{
		recycled.PushBack(id);
	}

	Referable* LookUp(u32 id)
	{
		if(id == 0)
			return nullptr;
		id--;
		if(id >= objects.Size())
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
	m_List(LUX_NEW(IDList))
{
}

IDManager::~IDManager()
{
	LUX_FREE(m_List);
}

ID IDManager::Create(Referable* obj)
{
	u32 value = m_List->Get(obj);
	return ID(value);
}

void IDManager::Release(ID id)
{
	u32 value = id.GetValue();
	m_List->Release(value);
}

Referable* IDManager::LookUp(ID id)
{
	return m_List->LookUp(id.GetValue());
}

} // namespace core
} // namespace lux