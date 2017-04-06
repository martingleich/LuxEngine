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

	virtual EResult Connect();
	virtual void Disconnect();
	virtual bool IsConnected() const;
	virtual EResult Aquire();
	virtual void UnAquire();
	virtual bool IsAquired() const;
	virtual bool IsForeground() const;
	virtual void Configure(bool isForeground);

protected:
	struct Button
	{
		bool state;
		string name;
		EElementType type;
	};

	struct Axis
	{
		int state;
		string name;
		EElementType type;
	};

	struct Area
	{
		math::vector2i state;
		string name;
		EElementType type;
	};

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