#ifndef INCLUDED_KEYBOARD_H
#define INCLUDED_KEYBOARD_H
#include "InputDeviceNull.h"
#include "core/lxArray.h"

namespace lux
{
namespace input
{

class KeyboardDevice : public InputDeviceNull
{
public:
	KeyboardDevice(const DeviceCreationDesc* desc, InputSystem* system);
	void Reset();
	void DisconnectReporting(InputSystem* system);
	EEventSource GetType() const;
	const event::Button* GetButton(int buttonCode) const;
	const event::Axis* GetAxis(int axisCode) const;
	const event::Area* GetArea(int areaCode) const;
	bool Update(Event& event);

	const core::String& GetElementName(EEventType type, int code) const;
	EElementType GetElementType(EEventType type, int id) const;
	int GetElementCount(EEventType type) const;

private:
	core::Array<ButtonElement> m_Buttons;
};

}
}

#endif // !INCLUDED_KEYBOARD_H