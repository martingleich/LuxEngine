#ifndef INCLUDED_LUX_MOUSE_H
#define INCLUDED_LUX_MOUSE_H
#include "InputDeviceNull.h"
#include "core/lxArray.h"

namespace lux
{
namespace input
{

class MouseDevice : public InputDeviceNull
{
public:
	MouseDevice(const DeviceCreationDesc* desc, InputSystem* system);
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
	core::Array<AxisElement> m_Axes;
	AreaElement m_Pos;
};

}
}

#endif // !INCLUDED_LUX_MOUSE_H