#include "InputSystemImpl.h"
#include "InputDeviceKeyboard.h"
#include "InputDeviceMouse.h"
#include "InputDeviceJoystick.h"

namespace lux
{
namespace input
{

InputSystemImpl::InputSystemImpl() :
	m_IsForeground(false),
	m_DefaultForegroundHandling(true)
{
}

core::Signal<const Event&>& InputSystemImpl::GetEventSignal()
{
	return m_EventSignal;
}

StrongRef<InputDevice> InputSystemImpl::CreateDevice(const DeviceCreationDesc* desc)
{
	InputDevice* device = nullptr;

	auto it = m_GUIDMap.Find(desc->GetGUID());
	if(it != m_GUIDMap.End()) {
		device = *it;
		device->Reset();
		return device;
	}

	switch(desc->GetType()) {
	case EEventSource::Keyboard:
		device = LUX_NEW(KeyboardDevice)(desc, this);
		break;
	case EEventSource::Mouse:
		device = LUX_NEW(MouseDevice)(desc, this);
		break;
	case EEventSource::Joystick:
		device = LUX_NEW(JoystickDevice)(desc, this);
		break;
	default:
		throw core::GenericInvalidArgumentException("desc", "Isn't a valid input device");
	}

	device->Configure(m_DefaultForegroundHandling);

	m_GUIDMap.Set(desc->GetGUID(), device);

	return device;
}

void InputSystemImpl::Update(Event& event)
{
	if(event.device) {
		event.device->Connect();
		if(m_KeyboardDevice && !m_KeyboardDevice->IsConnected() && event.device->GetType() == EEventSource::Keyboard) {
			m_KeyboardDevice = event.device;
		}

		if(event.device->Update(event) && event.device->IsAquired())
			m_EventSignal.Broadcast(event);
	}
}

void InputSystemImpl::SendUserEvent(const Event& event)
{
	m_EventSignal.Broadcast(event);
}

void InputSystemImpl::SetForegroundState(bool isForeground)
{
	m_IsForeground = isForeground;

	for(auto it = m_GUIDMap.First(); it != m_GUIDMap.End(); ++it) {
		StrongRef<InputDevice>& dev = *it;
		if(isForeground)
			dev->Aquire();
		else if(dev->IsForeground())
			dev->UnAquire();
	}
}

bool InputSystemImpl::IsForeground() const
{
	return m_IsForeground;
}

void InputSystemImpl::SetDefaultForegroundHandling(bool isForeground)
{
	if(isForeground == m_DefaultForegroundHandling)
		return;

	for(auto it = m_GUIDMap.First(); it != m_GUIDMap.End(); ++it) {
		if((*it)->IsForeground() == m_DefaultForegroundHandling)
			(*it)->Configure(isForeground);
	}

	m_DefaultForegroundHandling = isForeground;
}

bool InputSystemImpl::GetDefaultForegroundHandling() const
{
	return m_DefaultForegroundHandling;
}

StrongRef<InputDevice> InputSystemImpl::GetKeyboard()
{
	if(m_KeyboardDevice)
		return m_KeyboardDevice;

	InputDevice* firstNotConnected = nullptr;
	for(auto it = m_GUIDMap.First(); it != m_GUIDMap.End(); ++it) {
		if((*it)->GetType() == EEventSource::Keyboard) {
			firstNotConnected = *it;
			if((*it)->IsConnected()) {
				m_KeyboardDevice = it->GetWeak();
				break;
			}
		}
	}

	if(!m_KeyboardDevice)
		m_KeyboardDevice = firstNotConnected;

	return m_KeyboardDevice;
}

StrongRef<InputDevice> InputSystemImpl::GetMouse()
{
	if(m_MouseDevice)
		return m_MouseDevice;

	InputDevice* firstNotConnected = nullptr;
	for(auto it = m_GUIDMap.First(); it != m_GUIDMap.End(); ++it) {
		if((*it)->GetType() == EEventSource::Mouse) {
			firstNotConnected = *it;
			if((*it)->IsConnected()) {
				m_MouseDevice = it->GetWeak();
				break;
			}
		}
	}

	if(!m_MouseDevice)
		m_MouseDevice = firstNotConnected;

	return m_MouseDevice;
}

} // namespace input
} // namespace lux
