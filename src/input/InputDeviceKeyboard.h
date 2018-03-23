#ifndef INCLUDED_LUX_KEYBOARD_H
#define INCLUDED_LUX_KEYBOARD_H
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
	const core::Button* GetButton(int buttonCode) const;
	const core::Axis* GetAxis(int axisCode) const;
	const core::Area* GetArea(int areaCode) const;
	bool Update(Event& event);

	const core::String& GetElementName(EEventType type, int code) const;
	EElementType GetElementType(EEventType type, int id) const;
	int GetElementCount(EEventType type) const;

private:
	core::Array<ButtonElement> m_Buttons;
};

}
}

#endif // !INCLUDED_LUX_KEYBOARD_H