#ifndef INCLUDED_LUXDEVICE_WIN32_H
#define INCLUDED_LUXDEVICE_WIN32_H
#ifdef LUX_WINDOWS
#include "LuxEngine/LuxDevice.h"

#include "math/Dimension2.h"

#include "core/lxAlgorithm.h"
#include "core/lxArray.h"
#include "core/lxMemory.h"

#include "StrippedWindows.h"
#include "gui/WindowWin32.h"
#include "LuxSystemInfoWin32.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/RendererD3D9.h"
#include "video/d3d9/AdapterInformationD3D9.h"
#endif

namespace lux
{

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

	void BuildWindow(u32 width, u32 height, const String& title);
	void BuildInputSystem(bool isForeground = true);

	void BuildVideoDriver(const video::DriverConfig& config);

	core::Array<video::EDriverType> GetDriverTypes();
	StrongRef<video::AdapterList> GetVideoAdapters(video::EDriverType driver);

	void BuildImageSystem();
	void BuildSceneManager();
	void BuildGUIEnvironment();
	void BuildAll(const video::DriverConfig& config);

	bool WaitForWindowChange();
	bool Run();

	void RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop);

	void Sleep(u32 millis);

	StrongRef<gui::Window> GetWindow() const
	{
		return m_Window;
	}

	StrongRef<scene::SceneManager> GetSceneManager() const
	{
		return m_SceneManager;
	}

	StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const
	{
		return m_GUIEnv;
	}

	StrongRef<LuxSystemInfo> GetSystemInfo() const
	{
		return m_SystemInfo;
	}

	bool RunMessageQueue();
	bool HandleMessages(
		HWND wnd,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

private:
	StrongRef<gui::WindowWin32> m_Window;

	StrongRef<scene::SceneManager> m_SceneManager;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
#endif

	StrongRef<LuxSystemInfo> m_SystemInfo;

#ifdef LUX_COMPILE_WITH_D3D9
	UnknownRefCounted<IDirect3D9> m_D3D9;
#endif
};

} // namespace lux

#endif // LUX_WINDOWS
#endif
