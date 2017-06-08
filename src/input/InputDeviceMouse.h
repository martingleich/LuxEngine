#ifndef INCLUDED_MOUSE_H
#define INCLUDED_MOUSE_H
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
	const Button* GetButton(u32 buttonCode) const;
	const Axis* GetAxis(u32 axisCode) const;
	const Area* GetArea(u32 areaCode) const;
	bool Update(Event& event);

	const string& GetElementName(EEventType type, u32 code) const;
	EElementType GetElementType(EEventType type, u32 id) const;
	size_t GetElementCount(EEventType type) const;

private:
	core::array<ButtonElement> m_Buttons;
	core::array<AxisElement> m_Axes;
	AreaElement m_Pos;
};

}
}

#endif // !INCLUDED_MOUSE_H