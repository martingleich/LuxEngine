#include "InputSystemImpl.h"
#include "InputDeviceKeyboard.h"
#include "InputDeviceMouse.h"
#include "InputDeviceJoystick.h"

namespace lux
{
namespace input
{

InputSystemImpl::InputSystemImpl() :
	m_IsForeground(false)
{
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
		assertNeverReach("Event source isn't a input device.");
		return nullptr;
	}

	m_GUIDMap.Set(desc->GetGUID(), device);

	device->Aquire();

	return device;
}

void InputSystemImpl::SendUserEvent(const Event& event)
{
	if(m_Receiver) {
		m_Receiver->OnEvent(event);
	}
}

void InputSystemImpl::Update(Event& event)
{
	if(event.device) {
		event.device->Connect();
		if(m_KeyboardDevice && !m_KeyboardDevice->IsConnected() && event.device->GetType() == EEventSource::Keyboard) {
			m_KeyboardDevice = event.device;
		}

		if(event.device->Update(event) && m_Receiver)
			m_Receiver->OnEvent(event);
	}
}

void InputSystemImpl::SetInputReceiver(EventReceiver* receiver)
{
	m_Receiver = receiver;
}

EventReceiver* InputSystemImpl::GetInputReceiver() const
{
	return m_Receiver;
}

EResult InputSystemImpl::AquireDevice(InputDevice* device)
{
	if(device->IsForeground())
		return m_IsForeground ? EResult::Succeeded : EResult::Failed;

	return EResult::Succeeded;
}

EResult InputSystemImpl::UnAquireDevice(InputDevice* device)
{
	LUX_UNUSED(device);
	return EResult::Succeeded;
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

}
}