#ifndef INCLUDED_LUX_DEVICE_WIN32_H
#define INCLUDED_LUX_DEVICE_WIN32_H
#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "LuxDeviceNull.h"
#include "gui/WindowWin32.h"
#include "LuxSystemInfoWin32.h"

namespace lux
{
namespace input
{
#ifdef LUX_COMPILE_WITH_RAW_INPUT
class RawInputReceiver;
#endif
}

struct Win32WindowClass : core::Uncopyable
{
public:
	HINSTANCE instance;
	const wchar_t* className;

	Win32WindowClass();
	~Win32WindowClass();
};

class LuxDeviceWin32 : public LuxDeviceNull
{
public:
	LuxDeviceWin32();
	~LuxDeviceWin32();

	void BuildWindow(int width, int height, core::StringView title) override;
	void BuildInputSystem(bool isForeground) override;

	bool WaitForWindowChange() override;
	bool Run(int waitTime) override;

	void Sleep(int millis) override;

	StrongRef<gui::Window> GetWindow() const override;
	StrongRef<LuxSystemInfo> GetSystemInfo() const override;
	StrongRef<gui::Cursor> GetCursor() const override;

public:
	bool RunMessageQueue(int waitTime);
	bool HandleMessages(
		HWND wnd,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

private:
	Win32WindowClass m_WindowClass;
	StrongRef<LuxSystemInfoWin32> m_SysInfo;
	StrongRef<gui::WindowWin32> m_Window;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
#endif

	HANDLE m_NeverSetEvent;
};

} // namespace lux

#endif // LUX_WINDOWS
#endif
