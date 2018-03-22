#include "InputDeviceMouse.h"
#include "input/InputSystem.h"
#include "core/StringConverter.h"

namespace lux
{
namespace input
{

MouseDevice::MouseDevice(const DeviceCreationDesc* desc, InputSystem* system) :
	InputDeviceNull(desc->GetName(), system)
{
	m_Buttons.Resize(desc->GetElementCount(EEventType::Button));
	m_Axes.Resize(desc->GetElementCount(EEventType::Axis));

	for(int i = 0; i < m_Buttons.Size(); ++i) {
		auto& button = m_Buttons[i];
		auto elemDesc = desc->GetElementDesc(EEventType::Button, i);
		button.type = EElementType::Input | EElementType::Button | EElementType::PushButton;
		button.name = elemDesc.name;

		if(button.name.IsEmpty())
			button.name = "Button-" + core::StringConverter::ToString(i);
	}

	for(int i = 0; i < m_Axes.Size(); ++i) {
		auto& axis = m_Axes[i];
		auto elemDesc = desc->GetElementDesc(EEventType::Axis, i);
		axis.type = EElementType::Input | EElementType::Axis | EElementType::Rel;
		axis.name = elemDesc.name;

		if(axis.name.IsEmpty())
			axis.name = "Axis-" + core::StringConverter::ToString(i);
	}

	m_Pos.type = EElementType::Input | EElementType::Area | EElementType::Rel;
	m_Pos.name = desc->GetElementDesc(EEventType::Area, 0).name;
	if(m_Pos.name.IsEmpty())
		m_Pos.name = "Position";

	Reset();
}

void MouseDevice::Reset()
{
	for(int i = 0; i < m_Buttons.Size(); ++i)
		m_Buttons[i].state = false;
	for(int i = 0; i < m_Axes.Size(); ++i)
		m_Axes[i].state = 0;
	m_Pos.state.x = m_Pos.state.y = 0;
}

void MouseDevice::DisconnectReporting(InputSystem* system)
{
	Event event;
	event.device = this;
	event.source = EEventSource::Mouse;
	event.internal_abs_only = false;
	event.internal_rel_only = false;

	event.type = EEventType::Button;
	event.button.pressedDown = false;
	event.button.state = false;
	for(int i = 0; i < m_Buttons.Size(); ++i) {
		if(m_Buttons[i].state) {
			event.button.code = EKeyCode(i);
			system->SendUserEvent(event);
		}
	}

	event.type = EEventType::Axis;
	event.axis.abs = 0;
	for(int i = 0; i < m_Axes.Size(); ++i) {
		if(m_Axes[i].state && !TestFlag(m_Axes[i].type, EElementType::Rel)) {
			event.axis.code = EAxisCode(i);
			event.axis.rel = -m_Axes[i].state;
			system->SendUserEvent(event);
		}
	}

	// Don't reset the relative mouse-position.
}

EEventSource MouseDevice::GetType() const
{
	return EEventSource::Mouse;
}

const event::Button* MouseDevice::GetButton(int buttonCode) const
{
	return &m_Buttons.At(buttonCode);
}

const event::Axis* MouseDevice::GetAxis(int axisCode) const
{
	return &m_Axes.At(axisCode);
}

const event::Area* MouseDevice::GetArea(int areaCode) const
{
	if(areaCode != 0)
		throw core::OutOfRangeException();

	return &m_Pos;
}

bool MouseDevice::Update(Event& event)
{
	if(event.type == EEventType::Button) {
		if((int)event.button.code >= m_Buttons.Size())
			return false; // Silent ignore

		if(m_Buttons[event.button.code].state != event.button.state) {
			m_Buttons[event.button.code].state = event.button.state;
			return true;
		}
		return false;
	}

	if(event.type == EEventType::Axis) {
		if((int)event.axis.code >= m_Axes.Size())
			return false; // Silent ignore

		if(event.internal_abs_only)
			event.axis.rel = m_Axes[event.axis.code].state - event.axis.abs;
		else if(event.internal_rel_only)
			event.axis.abs = m_Axes[event.axis.code].state + event.axis.rel;

		if(event.axis.rel != 0.0) {
			m_Axes[event.axis.code].state += event.axis.rel;
			return true;
		}

		return false;
	}

	if(event.type == EEventType::Area) {
		if(event.area.code != 0)
			return false; // Silent ignore

		if(event.internal_abs_only) {
			event.area.relX = m_Pos.state.x - event.area.absX;
			event.area.relY = m_Pos.state.y - event.area.absY;
		} else if(event.internal_rel_only) {
			event.area.absX = m_Pos.state.x + event.area.relX;
			event.area.absY = m_Pos.state.y + event.area.relY;
		}

		if(m_Pos.state.x != event.area.absX || m_Pos.state.y != event.area.absY) {
			m_Pos.state.x = event.area.absX;
			m_Pos.state.y = event.area.absY;
			return true;
		}

		return false;
	}

	return false;
}

const core::String& MouseDevice::GetElementName(EEventType type, int code) const
{
	static core::String unknown = "(unknown)";
	if(type == EEventType::Button && code < m_Buttons.Size())
		return m_Buttons[code].name;

	if(type == EEventType::Axis && code < m_Axes.Size())
		return m_Axes[code].name;

	if(type == EEventType::Area && code == 0)
		return m_Pos.name;

	return unknown;
}

EElementType MouseDevice::GetElementType(EEventType type, int id) const
{
	if(type == EEventType::Button) {
		return m_Buttons.At(id).type;
	} else if(type == EEventType::Axis) {
		return m_Axes.At(id).type;
	} else if(type == EEventType::Area) {
		if(id != 0)
			throw core::OutOfRangeException();
		return m_Pos.type;
	} else {
		return EElementType::Other;
	}
}

int MouseDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return m_Buttons.Size();
	else if(type == EEventType::Axis)
		return m_Axes.Size();
	else if(type == EEventType::Area)
		return 1;
	else
		return 0;
}

}
}
