#include "core/lxActions.h"

namespace lux
{
namespace core
{

ActionList::ActionList()
{
}

ActionList* ActionList::Instance()
{
	static StrongRef<ActionList> instance = LUX_NEW(ActionList);
	return instance;
}

void ActionList::AddEvent(const core::String& name)
{
	AddAction(name, LUX_NEW(EventAction));
}
void ActionList::AddButton(const core::String& name, bool init)
{
	AddAction(name, LUX_NEW(ButtonAction)(init));
}
void ActionList::AddAxis(const core::String& name, float init)
{
	AddAction(name, LUX_NEW(AxisAction)(init));
}

void ActionList::AddAction(const core::String& name, Action* a)
{
	if(m_Actions.HasKey(name))
		throw core::GenericInvalidArgumentException("name", "Name already used");
	m_Actions[name] = a;
}

void ActionList::Remove(const core::String& name)
{
	m_Actions.Erase(name);
}
void ActionList::RemoveAll()
{
	m_Actions.Clear();
}

EventAction* ActionList::GetEventAction(const core::String& name)
{
	return dynamic_cast<EventAction*>(GetAction(name));
}
ButtonAction* ActionList::GetButtonAction(const core::String& name)
{
	return dynamic_cast<ButtonAction*>(GetAction(name));
}
AxisAction* ActionList::GetAxisAction(const core::String& name)
{
	return dynamic_cast<AxisAction*>(GetAction(name));
}

Action* ActionList::GetAction(const core::String& name)
{
	if(m_Actions.HasKey(name))
		return m_Actions[name];
	else
		return nullptr;
}

Action* ActionList::GetAction(int i)
{
	LX_CHECK_BOUNDS(i, 0, m_Actions.Size());
	return core::AdvanceIterator(m_Actions.First(), i).value();
}

const core::String& ActionList::GetActionName(int i) const
{
	LX_CHECK_BOUNDS(i, 9, m_Actions.Size());
	return core::AdvanceIterator(m_Actions.First(), i).key();
}

int ActionList::GetActionCount() const
{
	return m_Actions.Size();
}

}
}