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

namespace lux
{

namespace input
{
#ifdef LUX_COMPILE_WITH_RAW_INPUT
class RawInputReceiver;
#endif
}

class LuxDeviceWin32 : public LuxDevice, public gui::WindowWin32MsgCallback
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

	bool Run(float& fNumSecsPassed);

	bool OnMsg(
		UINT uiMessage,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

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


private:
	StrongRef<gui::Window> m_Window;

	StrongRef<scene::SceneManager> m_SceneManager;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
#endif
};

} // namespace lux

#endif // LUX_WINDOWS
#endif
