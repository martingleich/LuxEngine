#ifndef INCLUDED_ACTION_MAPPER_H
#define INCLUDED_ACTION_MAPPER_H
#include "core/lxSignal.h"
#include "core/lxEvent.h"
#include "core/lxString.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace core
{

class Action : public ReferenceCounted
{
public:
	virtual ~Action() {}

	virtual void FireEvent() = 0;
	virtual void FireButton(bool v) = 0;
	virtual void FireAxis(float v) = 0;
};

class EventAction : public Action
{
public:
	Signal<> signal;

	void FireEvent()
	{
		signal.Broadcast();
	}
	void FireButton(bool) { EventAction::FireEvent(); }
	void FireAxis(float) { EventAction::FireEvent(); }
};

class ButtonAction : public Action
{
public:
	Signal<bool> signal;

	ButtonAction(bool init)
	{
		state.state = init;
	}

	void FireEvent()
	{
		FireButton(true);
	}
	void FireButton(bool v)
	{
		signal.Broadcast(v);
		state.state = v;
	}
	void FireAxis(float v)
	{
		FireButton(v >= 0.5f);
	}

	const core::Button* GetButton() const
	{
		return &state;
	}

	bool GetState() const
	{
		return state.state;
	}

private:
	core::Button state;
};

class AxisAction : public Action
{
public:
	Signal<float> signal;

	AxisAction(float init)
	{
		state.state = init;
	}

	void FireEvent()
	{
		FireAxis(1.0f);
	}
	void FireButton(bool b)
	{
		FireAxis(b ? 1.0f : 0.0f);
	}
	void FireAxis(float v)
	{
		signal.Broadcast(v);
		state.state = v;
	}

	const core::Axis* GetAxis() const
	{
		return &state;
	}

	float GetState() const
	{
		return state.state;
	}

private:
	core::Axis state;
};

class ActionList : public ReferenceCounted
{
public:
	LUX_API static ActionList* Instance();

	LUX_API void AddEvent(const core::String& name);
	LUX_API void AddButton(const core::String& name, bool init = false);
	LUX_API void AddAxis(const core::String& name, float init = 0);

	LUX_API void AddAction(const core::String& name, Action* a);

	LUX_API void Remove(const core::String& name);
	LUX_API void RemoveAll();

	LUX_API EventAction* GetEventAction(const core::String& name);
	LUX_API ButtonAction* GetButtonAction(const core::String& name);
	LUX_API AxisAction* GetAxisAction(const core::String& name);

	LUX_API Action* GetAction(const core::String& name);

	LUX_API Action* GetAction(int i);

	LUX_API const core::String& GetActionName(int i) const;

	LUX_API int GetActionCount() const;

private:
	ActionList();
private:
	core::HashMap<core::String, StrongRef<Action>> m_Actions;
};

} // namespace event
} // namespace lux

#endif // #ifndef INCLUDED_ACTION_MAPPER_H