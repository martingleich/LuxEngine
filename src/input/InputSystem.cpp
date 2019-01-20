#include "input/InputSystem.h"

#include "input/InputDevice.h"

namespace lux
{
namespace input
{

static StrongRef<InputSystem> g_InputSystem;

void InputSystem::Initialize()
{
	g_InputSystem = LUX_NEW(InputSystem);
}

InputSystem* InputSystem::Instance()
{
	return g_InputSystem;
}

void InputSystem::Destroy()
{
	g_InputSystem.Reset();
}

InputSystem::InputSystem() :
	m_IsForeground(false),
	m_ForegroundHandling(true)
{
}

InputSystem::~InputSystem()
{
}

core::Signal<const Event&>& InputSystem::GetEventSignal()
{
	return m_EventSignal;
}

StrongRef<InputDevice> InputSystem::FindDevice(InputDeviceDesc* desc)
{
	auto it = m_GUIDMap.Find(desc->GetGUID());
	if(it != m_GUIDMap.end())
		return it->value;

	StrongRef<InputDevice> device = LUX_NEW(InputDevice)(desc, this);

	m_GUIDMap.Set(desc->GetGUID(), device);

	return device;
}

void InputSystem::Update(Event& event)
{
	if(!event.device)
		return;

	// If there is no default mouse or keyboard device, and the event was sent
	// from a mouse or keyboard use them as default device.
	if(event.device->GetType() == EDeviceType::Keyboard) {
		if(!m_KeyboardDevice)
			m_KeyboardDevice = event.device;
	} else if(event.device->GetType() == EDeviceType::Mouse) {
		if(!m_MouseDevice)
			m_MouseDevice = event.device;
	}

	bool changed = event.device->Update(event);
	if(!changed)
		return;
	if(!IsForeground() && !GetForegroundHandling())
		return;
	m_EventSignal.Broadcast(event);
}

void InputSystem::SetForegroundState(bool isForeground)
{
	m_IsForeground = isForeground;
}

bool InputSystem::IsForeground() const
{
	return m_IsForeground;
}

void InputSystem::SetForegroundHandling(bool isForeground)
{
	m_ForegroundHandling = isForeground;
}

bool InputSystem::GetForegroundHandling() const
{
	return m_ForegroundHandling;
}

StrongRef<InputDevice> InputSystem::GetKeyboard()
{
	if(m_KeyboardDevice)
		return m_KeyboardDevice;

	for(auto it = m_GUIDMap.begin(); it != m_GUIDMap.end(); ++it) {
		if(it->value->GetType() == EDeviceType::Keyboard) {
			m_KeyboardDevice = it->value.GetWeak();
			break;
		}
	}

	return m_KeyboardDevice;
}

StrongRef<InputDevice> InputSystem::GetMouse()
{
	if(m_MouseDevice)
		return m_MouseDevice;

	for(auto it = m_GUIDMap.begin(); it != m_GUIDMap.end(); ++it) {
		if(it->value->GetType() == EDeviceType::Mouse) {
			m_MouseDevice = it->value.GetWeak();
			break;
		}
	}

	return m_MouseDevice;
}
}
}
