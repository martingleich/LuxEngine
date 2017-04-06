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
	bool GetButtonState(u32 buttonCode) const;
	int GetAxisState(u32 axisCode) const;
	math::vector2i GetAreaState(u32 areaCode) const;
	bool Update(Event& event);

	const string& GetElementName(EEventType type, u32 code) const;

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
	core::array<Button> m_Buttons;
	core::array<Axis> m_Axes;
	
	//std::vector<Calibration> m_Calibration(m_Axes.size());
};

}
}


#endif // !INCLUDED_JOYSTICK_H