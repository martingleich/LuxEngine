#include "events/lxActions.h"

namespace lux
{
namespace event
{

ActionList::ActionList()
{
}

ActionList* ActionList::Instance()
{
	static StrongRef<ActionList> instance = LUX_NEW(ActionList);
	return instance;
}

void ActionList::AddEvent(const String& name)
{
	AddAction(name, LUX_NEW(EventAction));
}
void ActionList::AddButton(const String& name, bool init)
{
	AddAction(name, LUX_NEW(ButtonAction)(init));
}
void ActionList::AddAxis(const String& name, float init)
{
	AddAction(name, LUX_NEW(AxisAction)(init));
}

void ActionList::AddAction(const String& name, Action* a)
{
	if(m_Actions.HasKey(name))
		throw core::InvalidArgumentException("name", "Name already used");
	m_Actions[name] = a;
}

void ActionList::Remove(const String& name)
{
	m_Actions.Erase(name);
}
void ActionList::RemoveAll()
{
	m_Actions.Clear();
}

EventAction* ActionList::GetEventAction(const String& name)
{
	return dynamic_cast<EventAction*>(GetAction(name));
}
ButtonAction* ActionList::GetButtonAction(const String& name)
{
	return dynamic_cast<ButtonAction*>(GetAction(name));
}
AxisAction* ActionList::GetAxisAction(const String& name)
{
	return dynamic_cast<AxisAction*>(GetAction(name));
}

Action* ActionList::GetAction(const String& name)
{
	if(m_Actions.HasKey(name))
		return m_Actions[name];
	else
		return nullptr;
}

Action* ActionList::GetAction(size_t i)
{
	if(i >= m_Actions.Size())
		throw core::OutOfRangeException();
	return core::AdvanceIterator(m_Actions.First(), i).value();
}

const String& ActionList::GetActionName(size_t i) const
{
	if(i >= m_Actions.Size())
		throw core::OutOfRangeException();
	return core::AdvanceIterator(m_Actions.First(), i).key();
}

size_t ActionList::GetActionCount() const
{
	return m_Actions.Size();
}

}
}