#ifndef INCLUDED_JOYSTICK_H
#define INCLUDED_JOYSTICK_H
#include "InputDeviceNull.h"
#include "core/lxArray.h"

namespace lux
{
namespace input
{

class JoystickDevice : public InputDeviceNull
{
public:
	JoystickDevice(const DeviceCreationDesc* desc, InputSystem* system);
	void Reset();
	void DisconnectReporting(InputSystem* system);
	EEventSource GetType() const;
	const event::Button* GetButton(u32 buttonCode) const;
	const event::Axis* GetAxis(u32 axisCode) const;
	const event::Area* GetArea(u32 areaCode) const;
	bool Update(Event& event);

	const core::String& GetElementName(EEventType type, u32 code) const;

	EElementType GetElementType(EEventType type, u32 id) const;
	size_t GetElementCount(EEventType type) const;
	
	// TODO:
	/*
	struct Calibration
	{
		int min;
		int max;
		int deadZone;
		int saturation;
		
		static int DEFAULT_RANGE=1;
	};
	
	void SetCalibration(const Calibration& c, int mode=0);
	const Calibration& GetCalibration();
	
	void SetCalibration(ElementId axis, const Calibration& c, int mode=0).
	const Calibration& GetCalibration(ElementId axis);
	*/

private:
	core::Array<ButtonElement> m_Buttons;
	core::Array<AxisElement> m_Axes;
	
	//std::vector<Calibration> m_Calibration(m_Axes.size());
};

}
}


#endif // !INCLUDED_JOYSTICK_H