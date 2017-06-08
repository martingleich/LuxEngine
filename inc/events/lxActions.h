#ifndef INCLUDED_ACTION_MAPPER_H
#define INCLUDED_ACTION_MAPPER_H
#include "events/lxSignal.h"
#include "input/InputReceiver.h"
#include "input/InputDevice.h"
#include "core/lxString.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace events
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
	signal<> signal;

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
	signal<bool> signal;

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

	const input::Button* GetButton() const
	{
		return &state;
	}

	bool GetState() const
	{
		return state.state;
	}

private:
	input::Button state;
};

class AxisAction : public Action
{
public:
	signal<float> signal;

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

	const input::Axis* GetAxis() const
	{
		return &state;
	}

	float GetState() const
	{
		return state.state;
	}

private:
	input::Axis state;
};

/**
The default actions are:
Camera-Actions used by scene::CameraControl:
axis cam_forward: Strength of the camera moving forward, negative to move backward
axis cam_stride: Strength of the camera moving right, negative to move left
axis cam_up: Strength of the camera moving up, negative to move down

axis cam_look_x: Speed of the camera rotating around the y axis
axis cam_look_y: Speed of the camera rotation around the x axis
*/
class ActionList : public ReferenceCounted
{
public:
	LUX_API static ActionList* Instance();

	LUX_API void AddEvent(const string& name);
	LUX_API void AddButton(const string& name, bool init = false);
	LUX_API void AddAxis(const string& name, float init = 0);

	LUX_API void AddAction(const string& name, Action* a);

	LUX_API void Remove(const string& name);
	LUX_API void RemoveAll();

	LUX_API EventAction* GetEventAction(const string& name);
	LUX_API ButtonAction* GetButtonAction(const string& name);
	LUX_API AxisAction* GetAxisAction(const string& name);

	LUX_API Action* GetAction(const string& name);

	LUX_API Action* GetAction(size_t i);

	LUX_API const string& GetActionName(size_t i) const;

	LUX_API size_t GetActionCount() const;

private:
	ActionList();
private:
	core::HashMap<string, StrongRef<Action>> m_Actions;
};

}
}

#endif // #ifndef INCLUDED_ACTION_MAPPER_H