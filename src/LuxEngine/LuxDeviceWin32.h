#ifndef INCLUDED_LUXDEVICE_WIN32_H
#define INCLUDED_LUXDEVICE_WIN32_H
#include "LuxEngine/LuxDevice.h"

#include "math/Dimension2.h"

#include "core/lxAlgorithm.h"
#include "core/lxArray.h"
#include "core/lxMemory.h"

#include "gui/Window.h"

#ifdef LUX_WINDOWS
#include "StrippedWindows.h"

namespace lux
{
namespace gui
{
class GUIEnvironment;
}

namespace input
{
#ifdef LUX_COMPILE_WITH_RAW_INPUT
class RawInputReceiver;
#endif
}

class LuxDeviceWin32 : public LuxDevice
{
public:
	LuxDeviceWin32();
	~LuxDeviceWin32();

	void BuildWindow(u32 Width, u32 height, const String& stitle);
	void SetOwnWindow(void* hOwnWindow);

	void BuildInputSystem(bool isForeground = true);

	void BuildVideoDriver(const video::DriverConfig& config);
	void BuildImageSystem();
	void BuildSceneManager();
	void BuildGUIEnvironment();
	void BuildAll(const video::DriverConfig& config);

	HWND CreateNewWindow(u32 width, u32 height, const String& title);
	void CloseDevice();
	bool HandleSystemMessages();
	bool Run(float& fNumSecsPassed);

	double GetTime() const;
	StrongRef<gui::Window> GetWindow() const;

	StrongRef<scene::SceneManager> GetSceneManager() const;
	StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const;

	LRESULT WinProc(HWND window,
		UINT uiMessage,
		WPARAM WParam,
		LPARAM LParam);

private:
	static LRESULT WINAPI WindowProc(HWND window,
		UINT uiMessage,
		WPARAM WParam,
		LPARAM LParam);

private:
	struct WindowCallback : public gui::WindowEventCallback
	{
	public:
		WindowCallback(LuxDeviceWin32* dev) :
			device(dev)
		{
		}

		void OnClose(gui::Window& window)
		{
			LUX_UNUSED(window);

			if(device)
				device->CloseDevice();
		}

	private:
		LuxDeviceWin32* device;
	};

	WindowCallback m_WindowCallback;

	bool m_OwnWindow;
	StrongRef<gui::Window> m_Window;

	bool m_IsAppActive;
	bool m_Quit;

	double m_Time;

	StrongRef<scene::SceneManager> m_SceneManager;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;

	const wchar_t* const m_LuxWindowClassName;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
#endif
};

}    //namespace lux

#endif // LUX_WINDOWS
#endif
