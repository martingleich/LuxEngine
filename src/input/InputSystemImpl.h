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

	events::signal<const Event&>& GetEventSignal();

	void Update(Event& event);
	void SendUserEvent(const Event& event);

	StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc);

	void SetForegroundState(bool isForeground);
	bool IsForeground() const;

	void SetDefaultForegroundHandling(bool isForeground);
	bool GetDefaultForegroundHandling() const;

	StrongRef<InputDevice> GetKeyboard();

private:
	core::HashMap<string, StrongRef<InputDevice>> m_GUIDMap;
	WeakRef<InputDevice> m_KeyboardDevice;

	events::signal<const input::Event&> m_EventSignal;

	bool m_IsForeground;
	bool m_DefaultForegroundHandling;
};

}
}

#endif // !INCLUDED_INPUT_SYSTEM_H