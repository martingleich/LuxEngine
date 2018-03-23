#ifndef INCLUDED_LUX_INPUT_DEVICE_NULL_H
#define INCLUDED_LUX_INPUT_DEVICE_NULL_H
#include "input/InputDevice.h"

namespace lux
{
namespace input
{

class InputDeviceNull : public InputDevice
{
public:
	InputDeviceNull(const core::String& name, InputSystem* sys);
	virtual ~InputDeviceNull() {}

	virtual const core::String& GetName() const;

	virtual bool Connect();
	virtual void Disconnect();
	virtual bool IsConnected() const;
	virtual bool Aquire();
	virtual void UnAquire();
	virtual bool IsAquired() const;
	virtual bool IsForeground() const;
	virtual void Configure(bool isForeground);

protected:
	struct ElementData
	{
		core::String name;
		EElementType type;
	};

	struct ButtonElement : core::Button, ElementData {};

	struct AxisElement : core::Axis, ElementData {};

	struct AreaElement : core::Area, ElementData {};

private:
	core::String m_Name;
	bool m_Connected;
	bool m_Aquired;

	bool m_Foreground;

	StrongRef<InputSystem> m_System;
};

}
}

#endif // #ifndef INCLUDED_LUX_INPUT_DEVICE_NULL_H