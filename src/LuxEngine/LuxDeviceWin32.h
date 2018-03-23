#ifndef INCLUDED_LUX_DEVICE_WIN32_H
#define INCLUDED_LUX_DEVICE_WIN32_H
#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "LuxDeviceNull.h"
#include "gui/WindowWin32.h"

namespace lux
{
namespace input
{
#ifdef LUX_COMPILE_WITH_RAW_INPUT
class RawInputReceiver;
#endif
}

class LuxDeviceWin32 : public LuxDeviceNull
{
public:
	LuxDeviceWin32();
	~LuxDeviceWin32();

	void BuildWindow(int width, int height, const core::String& title);
	void BuildInputSystem(bool isForeground = true);
	bool WaitForWindowChange();
	bool Run(int waitTime);

	void Sleep(int millis);

	StrongRef<gui::Window> GetWindow() const
	{
		return m_Window;
	}

	StrongRef<LuxSystemInfo> GetSystemInfo() const;
	StrongRef<gui::Cursor> GetCursor() const;

	bool RunMessageQueue(int waitTime);
	bool HandleMessages(
		HWND wnd,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

private:
	StrongRef<gui::WindowWin32> m_Window;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
#endif

	HANDLE m_NeverSetEvent;
};

} // namespace lux

#endif // LUX_WINDOWS
#endif
