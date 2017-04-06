#include "InputDeviceNull.h"
#include "input/InputSystem.h"

namespace lux
{
namespace input
{

InputDeviceNull::InputDeviceNull(const string& name, InputSystem* sys) :
	m_Name(name),
	m_Connected(false),
	m_Aquired(false),
	m_Foreground(true),
	m_System(sys)
{
}

const string& InputDeviceNull::GetName() const
{
	return m_Name;
}

EResult InputDeviceNull::Connect()
{
	m_Connected = true;
	return EResult::Succeeded;
}

void InputDeviceNull::Disconnect()
{
	DisconnectReporting(m_System);
	m_Connected = false;
}

bool InputDeviceNull::IsConnected() const
{
	return m_Connected;
}

EResult InputDeviceNull::Aquire()
{
	EResult result = m_System->AquireDevice(this);
	m_Aquired = Succeeded(result);
	return result;
}

void InputDeviceNull::UnAquire()
{
	EResult result = m_System->UnAquireDevice(this);
	LUX_UNUSED(result);
	m_Aquired = false;
}
bool InputDeviceNull::IsAquired() const
{
	return m_Aquired;
}
bool InputDeviceNull::IsForeground() const
{
	return m_Foreground;
}
void InputDeviceNull::Configure(bool isForeground)
{
	m_Foreground = isForeground;
}

}
}
