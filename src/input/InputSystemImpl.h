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
	InputSystemImpl(bool defaultForeground);

	void Update(Event& event);
	void SendUserEvent(const Event& event);
	void SetInputReceiver(EventReceiver* receiver);
	EventReceiver* GetInputReceiver() const;

	StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc);

	void SetForegroundState(bool isForeground);
	bool IsForeground() const;

	void SetDefaultForegroundHandling(bool isForeground);
	bool GetDefaultForegroundHandling() const;

	StrongRef<InputDevice> GetKeyboard();

private:
	core::HashMap<string, StrongRef<InputDevice>> m_GUIDMap;
	WeakRef<InputDevice> m_KeyboardDevice;

	EventReceiver* m_Receiver;

	bool m_IsForeground;
	bool m_DefaultForegroundHandling;
};

}
}

#endif // !INCLUDED_INPUT_SYSTEM_H