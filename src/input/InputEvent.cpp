#include "input/InputEvent.h"
#include "input/InputDevice.h"

namespace lux
{
namespace input
{

void EventHandler::OnEvent(const input::Event& e)
{
	switch(e.device->GetType()) {
	case EDeviceType::Keyboard:
		if(auto button = e.TryAs<ButtonEvent>())
			OnKey(button->pressedDown, button->code, e);
		break;
	case EDeviceType::Mouse:
		if(auto button = e.TryAs<ButtonEvent>()) {
			if(button->code == input::MOUSE_BUTTON_LEFT)
				OnLButton(button->pressedDown, e);
			else if(button->code == input::MOUSE_BUTTON_RIGHT)
				OnRButton(button->pressedDown, e);
		} else if(auto area = e.TryAs<AreaEvent>()) {
			if(area->code == MOUSE_AREA)
				OnMouseMove(area->rel, e);
		} else if(auto axis = e.TryAs<AxisEvent>()) {
			if(axis->code == input::MOUSE_AXIS_WHEEL)
				OnMouseWheel(axis->rel, e);
		}
		break;
	default:
		break;
	}
}

} // namespace input
} // namespace lux