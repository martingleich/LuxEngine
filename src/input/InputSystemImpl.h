#ifndef INCLUDED_INPUT_SYSTEM_IMPL_H
#define INCLUDED_INPUT_SYSTEM_IMPL_H
#include "input/InputSystem.h"

#include "core/lxHashMap.h"
#include "core/lxString.h"

namespace lux
{
namespace input
{

class InputSystemImpl : public InputSystem
{
public:
	InputSystemImpl();

	event::Signal<const Event&>& GetEventSignal();

	void Update(Event& event);
	void SendUserEvent(const Event& event);

	StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc);

	void SetForegroundState(bool isForeground);
	bool IsForeground() const;

	void SetDefaultForegroundHandling(bool isForeground);
	bool GetDefaultForegroundHandling() const;

	StrongRef<InputDevice> GetKeyboard();
	StrongRef<InputDevice> GetMouse();

private:
	core::HashMap<String, StrongRef<InputDevice>> m_GUIDMap;
	WeakRef<InputDevice> m_KeyboardDevice;
	WeakRef<InputDevice> m_MouseDevice;

	event::Signal<const input::Event&> m_EventSignal;

	bool m_IsForeground;
	bool m_DefaultForegroundHandling;
};

}
}

#endif // !INCLUDED_INPUT_SYSTEM_H