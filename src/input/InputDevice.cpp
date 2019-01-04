#include "input/InputDevice.h"
#include "input/InputSystem.h"

namespace lux
{
namespace input
{

InputDevice::InputDevice(InputDeviceDesc* desc, InputSystem* system)
{
	m_Desc = desc;
	m_System = system;
	m_Name = desc->GetName();
	m_Type = desc->GetType();

	int buttoncount = desc->GetElementCount(EDeviceEventType::Button);
	for(int i = 0; i < buttoncount; ++i) {
		ButtonElement elem;
		elem.type = desc->GetElementType(EDeviceEventType::Button, i);
		elem.state = false;
		m_Buttons.PushBack(elem);
	}
	int axiscount = desc->GetElementCount(EDeviceEventType::Axis);
	for(int i = 0; i < axiscount; ++i) {
		AxisElement elem;
		elem.type = desc->GetElementType(EDeviceEventType::Axis, i);
		elem.state = 0;
		m_Axes.PushBack(elem);
	}
	int areacount = desc->GetElementCount(EDeviceEventType::Area);
	for(int i = 0; i < areacount; ++i) {
		AreaElement elem;
		elem.type = desc->GetElementType(EDeviceEventType::Area, i);
		elem.state = math::Vector2F(0, 0);
		m_Areas.PushBack(elem);
	}
}

InputDevice::~InputDevice()
{
}

bool InputDevice::Update(Event& event)
{
	if(auto button = event.TryAs<ButtonEvent>()) {
		auto& elem = m_Buttons.At(button->code);
		if(elem.state != button->pressedDown) {
			elem.state = button->pressedDown;
			return true;
		}
		return false;
	}
	if(auto axis = event.TryAs<AxisEvent>()) {
		auto& elem = m_Axes.At(axis->code);
		if(!TestFlag(elem.type, EDeviceElementType::Rel) && TestFlag(elem.type, EDeviceElementType::Abs))
			axis->rel = elem.state - axis->abs;
		if(!TestFlag(elem.type, EDeviceElementType::Abs) && TestFlag(elem.type, EDeviceElementType::Rel))
			axis->abs = elem.state + axis->rel;

		if(elem.state != axis->abs) {
			elem.state = axis->abs;
			return true;
		}

		return false;
	}
	if(auto area = event.TryAs<AreaEvent>()) {
		auto& elem = m_Areas.At(area->code);
		if(!TestFlag(elem.type, EDeviceElementType::Rel) && TestFlag(elem.type, EDeviceElementType::Abs))
			area->rel = elem.state - area->abs;
		if(!TestFlag(elem.type, EDeviceElementType::Abs) && TestFlag(elem.type, EDeviceElementType::Rel))
			area->abs = elem.state + area->rel;

		if(elem.state != area->abs) {
			elem.state = area->abs;
			return true;
		}

		return false;
	}

	return false;
}

} // namespace input
} // namespace lux