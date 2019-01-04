#ifndef INCLUDED_LUX_JOYSTICK_H
#define INCLUDED_LUX_JOYSTICK_H
#include "input/InputDeviceNull.h"
#include "core/lxArray.h"

namespace lux
{
namespace input
{

class JoystickDevice : public InputDevice
{
public:
	JoystickDevice(const DeviceCreationDesc* desc, InputSystem* system);

	EDeviceType GetType() const override;

	bool GetButton(int buttonCode) const override;
	float GetAxis(int axisCode) const override;
	math::Vector2F GetArea(int areaCode) const override;

	const core::String& GetName() const { return m_Name; }

	void Connect() override;
	void Disconnect(bool reset) override;
	bool IsConnected() const override;

	bool Update(Event& event) override;

	const core::String& GetElementName(EDeviceEventType type, int code) const override;

	EDeviceElementType GetElementType(EDeviceEventType type, int id) const override;
	int GetElementCount(EDeviceEventType type) const override;

protected:
	void Reset();

private:
	core::Array<ButtonElement> m_Buttons;
	core::Array<AxisElement> m_Axes;

	bool m_IsConnected;

	core::String m_Name;
	StrongRef<InputSystem> m_System;
};

}
}


#endif // !INCLUDED_LUX_JOYSTICK_H