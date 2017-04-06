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
	bool GetButtonState(u32 buttonCode) const;
	int GetAxisState(u32 axisCode) const;
	math::vector2i GetAreaState(u32 areaCode) const;
	bool Update(Event& event);

	const string& GetElementName(EEventType type, u32 code) const;
	EElementType GetElementType(EEventType type, u32 id) const;
	size_t GetElementCount(EEventType type) const;

private:
	core::array<Button> m_Buttons;
	core::array<Axis> m_Axes;
	Area m_Pos;
};

}
}

#endif // !INCLUDED_MOUSE_H