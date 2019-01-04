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
	EDeviceType GetType() const;
	const core::Button* GetButton(int buttonCode) const;
	const core::Axis* GetAxis(int axisCode) const;
	const core::Area* GetArea(int areaCode) const;
	bool Update(Event& event);

	const core::String& GetElementName(EDeviceEventType type, int code) const;
	EDeviceElementType GetElementType(EDeviceEventType type, int id) const;
	int GetElementCount(EDeviceEventType type) const;

private:
	core::Array<ButtonElement> m_Buttons;
};

}
}

#endif // !INCLUDED_LUX_KEYBOARD_H