#ifndef INCLUDED_LUX_INPUT_SYSTEM_IMPL_H
#define INCLUDED_LUX_INPUT_SYSTEM_IMPL_H
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

	core::Signal<const Event&>& GetEventSignal();

	void Update(Event& event);

	StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc);

	void SetForegroundState(bool isForeground);
	bool IsForeground() const;

	void SetDefaultForegroundHandling(bool isForeground);
	bool GetDefaultForegroundHandling() const;

	StrongRef<InputDevice> GetKeyboard();
	StrongRef<InputDevice> GetMouse();
};

}
}

#endif // !INCLUDED_LUX_INPUT_SYSTEM_H