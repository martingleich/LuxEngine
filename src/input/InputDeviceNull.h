#ifndef INCLUDED_INPUT_DEVICE_NULL_H
#define INCLUDED_INPUT_DEVICE_NULL_H
#include "input/InputDevice.h"

namespace lux
{
namespace input
{

class InputDeviceNull : public InputDevice
{
public:
	InputDeviceNull(const string& name, InputSystem* sys);
	virtual ~InputDeviceNull() {}

	virtual const string& GetName() const;

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
		string name;
		EElementType type;
	};

	struct ButtonElement : Button, ElementData {};

	struct AxisElement : Axis, ElementData {};

	struct AreaElement : Area, ElementData {};

private:
	string m_Name;
	bool m_Connected;
	bool m_Aquired;

	bool m_Foreground;

	StrongRef<InputSystem> m_System;
};

}
}

#endif // #ifndef INCLUDED_INPUT_DEVICE_NULL_H