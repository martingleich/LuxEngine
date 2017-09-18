#ifndef INCLUDED_LUXDEVICE_H
#define INCLUDED_LUXDEVICE_H
#include "core/lxString.h"
#include "core/ReferenceCounted.h"
#include "video/DriverConfig.h"

namespace lux
{

namespace scene
{
class Scene;
class SceneRenderer;
}

namespace gui
{
class Window;
class Cursor;
class GUIEnvironment;
}
class LuxSystemInfo;

//! The main class for the engine
class LuxDevice : public ReferenceCounted
{
public:
	virtual ~LuxDevice()
	{
	}

	//! Create the window associated with the engine.
	/**
	Requires core device.
	\param width The width of the window.
	\param height The height of the window.
	\param title The title of the window.
	*/
	virtual void BuildWindow(u32 Width, u32 height, const String& title) = 0;

	//! Create the input system of the engine.
	/**
	Requires associated window.
	Initialized the input system of the engine.
	\param isForeground Are input events only sent when the window is in the foreground.
	*/
	virtual void BuildInputSystem(bool isForeground = true) = 0;

	//! Creates the video driver component of the engine.
	/**
	Requires associated window.
	After this call all video modules are fully usable.
	The image module is fully usable.
	\param config The config data for the video driver.
	\param user User parameter to be passed to the module factory
	*/
	virtual void BuildVideoDriver(const video::DriverConfig& config, void* user=nullptr) = 0;
	//! Get available driver types.
	virtual core::Array<String> GetDriverTypes() = 0;
	//! Get available adapters for a driver type.
	virtual StrongRef<video::AdapterList> GetVideoAdapters(const String& driver) = 0;

	//! Creates the scene component of the engine
	/**
	Requires video driver and the input system.
	After this call all scene modules are fully usable.
	Creates a single scene object.
	\param renderer The name of the renderer to use(pass empty string to use default renderer).
	\param user User parameter to be passed to the module factory.
	*/
	virtual void BuildScene(const String& renderer=nullptr, void* user=nullptr) = 0;
	virtual StrongRef<scene::Scene> CreateScene() = 0;

	//! Create the gui environment
	/**
	Requires a window, the video module and the input system
	*/
	virtual void BuildGUIEnvironment() = 0;

	//! Build the image system of the engine.
	/**
	Requires the core device.
	Is automatic called by BuildVideoDriver, but can also
	be called on it's own.
	*/
	virtual void BuildImageSystem() = 0;

	//! Build all components.
	/**
	The created window has the same resolution as the backbuffer.
	*/
	virtual void BuildAll(const video::DriverConfig& config) = 0;

	//! Runs all engine internal actions.
	/**
	\param [out] numSecsPassed The time in seconds since the last call to Run.
	\return Returns true if the device is still running, returns false once when the device was closed.
	*/
	virtual bool Run() = 0;

	//! Wait until the window changed.
	/**
	May return at any time.
	Returns false if window change can't be detected, otherwise true.
	*/
	virtual bool WaitForWindowChange() = 0;

	struct SimpleFrameLoop
	{
		float timeScale = 1.0f;
		bool pauseOnLostFocus = false;
		bool pauseOnMinimize = true;

		// Passed to postMoveProc, postRenderProc, preFrameProc
		void* userData = nullptr;
		// Run each frame
		// Default:
		//  Animates Scenemanager then GUI then postMoveProc
		//  Renderes Scenemanager then GUI then postRenderProc
		void(*frameProc)(float, void*) = nullptr;
		// Default empty
		void(*postMoveProc)(float, void*) = nullptr;
		// Default empty
		void(*postRenderProc)(float, void*) = nullptr;

		// Default handles:
		// Pause on minimize, lost focus
		// Driver restore on device loss
		// Return false to cancel frame
		bool(*preFrameProc)(void*) = nullptr;
	};
	
	//! Runs a simple frame loop.
	/**
	Performs a frame loop, call LuxDevice::Run, Renders and animates the gui and the scene, reset driver on failure.
	Pause the engine if the window is minimized and so on.
	\param loop Data for the frame loop.
	*/
	virtual void RunSimpleFrameLoop(const SimpleFrameLoop& loop) = 0;

	//! Pauses the calling process for some time
	virtual void Sleep(u32 millis) = 0;

	//! Get the main scene of the engine.
	virtual StrongRef<scene::Scene> GetScene() const = 0;
	//! Get the main scene renderer of the engine.
	virtual StrongRef<scene::SceneRenderer> GetSceneRenderer() const = 0;
	//! Get the main gui environment of the engine.
	virtual StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const = 0;

	//! Get the os window of the engine.
	virtual StrongRef<gui::Window> GetWindow() const = 0;
	//! Get the os cursor of the engine.
	virtual StrongRef<gui::Cursor> GetCursor() const = 0;

	//! Retrieve system information.
	virtual StrongRef<LuxSystemInfo> GetSystemInfo() const = 0;
};

//! Create a new lux device.
LUX_API StrongRef<LuxDevice> CreateDevice();

}    //namespace lux

#endif
