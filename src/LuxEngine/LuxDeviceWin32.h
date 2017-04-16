#ifndef INCLUDED_CLUXDEVICEWIN32_H
#define INCLUDED_CLUXDEVICEWIN32_H
#include "StrippedWindows.h"

#include "LuxEngine/LuxDevice.h"

#include "math/dimension2d.h"

#include "core/lxAlgorithm.h"
#include "core/lxArray.h"
#include "core/lxMemory.h"
#include "gui/Window.h"

namespace lux
{
namespace gui
{
class GUIEnvironment;
}

namespace io
{
class FileSystem;
}

namespace video
{
class ImageSystem;
}

namespace core
{
class Timer;
class ReferableFactory;
class ResourceSystem;
}

namespace input
{
class RawInputReceiver;
}

class LuxDeviceWin32 : public LuxDevice
{
public:
	LuxDeviceWin32();
	~LuxDeviceWin32();

	bool BuildCoreDevice();

	bool BuildWindow(u32 Width, u32 height, const string& stitle);
	void SetOwnWindow(void* hOwnWindow);

	bool BuildInputSystem(bool isForeground = true);

	bool BuildVideoDriver(const video::DriverConfig& config);
	bool BuildImageSystem();
	bool BuildSceneManager();
	bool BuildGUIEnvironment();
	bool BuildAll(const video::DriverConfig& config);

	HWND CreateNewWindow(u32 width, u32 height, const string& title);
	void CloseDevice();
	bool HandleSystemMessages();
	void PostEvent(const input::Event& event, input::EEventTarget target = input::EEventTarget::All);
	void SetUserEventReceiver(input::UserEventReceiver* receiver);
	StrongRef<input::UserEventReceiver> GetUserEventReceiver() const;
	bool Run(float& fNumSecsPassed);


	double GetTime() const;
	StrongRef<gui::Window> GetWindow() const;

	StrongRef<video::VideoDriver> GetDriver() const;
	StrongRef<scene::SceneManager> GetSceneManager() const;
	StrongRef<input::InputSystem> GetInputSystem() const;
	StrongRef<video::MaterialLibrary> GetMaterialLibrary() const;
	StrongRef<io::FileSystem> GetFileSystem() const;
	StrongRef<video::ImageSystem> GetImageSystem() const;
	StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const;
	StrongRef<core::Timer> GetTimer() const;
	StrongRef<core::ReferableFactory> GetReferableFactory() const;
	StrongRef<core::ResourceSystem> GetResourceSystem() const;
	StrongRef<scene::MeshSystem> GetMeshSystem() const;

	LRESULT WinProc(HWND window,
		UINT uiMessage,
		WPARAM WParam,
		LPARAM LParam);

private:
	static LRESULT WINAPI WindowProc(HWND window,
		UINT uiMessage,
		WPARAM WParam,
		LPARAM LParam);

	bool BuildMaterials();

private:
	struct InputEventProxy : public input::EventReceiver
	{
		InputEventProxy(LuxDeviceWin32* d) :
			device(d)
		{}

		bool OnEvent(const input::Event& event)
		{
			device->PostEvent(event, input::EEventTarget::All);
			return true;
		}

		LuxDeviceWin32* device;
	};

	struct WindowCallback : public gui::WindowEventCallback
	{
	public:
		WindowCallback(LuxDeviceWin32* dev) :
			device(dev)
		{}

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
	InputEventProxy m_InputEventProxy;

	bool m_OwnWindow;
	StrongRef<gui::Window> m_Window;

	bool m_IsAppActive;
	bool m_Quit;

	double m_Time;

	WeakRef<input::UserEventReceiver> m_UserEventReceiver;

	StrongRef<input::InputSystem> m_InputSystem;
	StrongRef<video::VideoDriver> m_Driver;
	StrongRef<video::ImageSystem> m_ImageSystem;
	StrongRef<scene::SceneManager> m_SceneManager;
	StrongRef<io::FileSystem> m_Filesystem;
	StrongRef<core::Timer> m_Timer;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;
	StrongRef<core::ReferableFactory> m_ReferableFactory;
	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<input::RawInputReceiver> m_RawInputReceiver;
	StrongRef<video::MaterialLibrary> m_MaterialLibrary;
	StrongRef<scene::MeshSystem> m_MeshSystem;

	const wchar_t* const m_LuxWindowClassName;
};

}    //namespace lux

#endif
