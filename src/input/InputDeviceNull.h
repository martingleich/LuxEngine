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

	virtual void Connect() override;
	virtual void Disconnect(bool reset) override;
	virtual bool IsConnected() const override;

protected:
	//! Is called when the device is disconnected from the input system
	virtual void DisconnectReporting(InputSystem* system) = 0;

protected:
	struct ElementData
	{
		core::String name;
		EDeviceElementType type;
	};

	struct ButtonElement : ElementData
	{
		bool state;
	};

	struct AxisElement : ElementData
	{
		float state;
	};

	struct AreaElement : ElementData
	{
		math::Vector2F state;
	};

private:
	core::String m_Name;
	StrongRef<InputSystem> m_System;
};

}
}

#endif // #ifndef INCLUDED_LUX_INPUT_DEVICE_NULL_H