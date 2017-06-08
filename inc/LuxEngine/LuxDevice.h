#ifndef INCLUDED_LUXDEVICE_H
#define INCLUDED_LUXDEVICE_H
#include "core/lxString.h"
#include "core/ReferenceCounted.h"

namespace lux
{

namespace input
{
class InputSystem;
}

namespace video
{
class VideoDriver;
class MaterialLibrary;
class ImageSystem;
struct DriverConfig;
class MeshSystem;
}

namespace scene
{
class SceneManager;
}

namespace gui
{
class Window;
class GUIEnvironment;
}


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
	virtual void BuildWindow(u32 Width, u32 height, const string& title) = 0;
	virtual void SetOwnWindow(void* OwnWindow) = 0;

	//! Create the input system of the engine.
	/**
	Requires associated window.
	Initialized the input system of the engine.
	\param isForeground Are input events only sent when the window is in the foreground.
	*/
	virtual void BuildInputSystem(bool isForeground=true) = 0;

	//! Creates the video driver component of the engine.
	/**
	Requires associated window.
	After this call all video modules are fully usable.
	The image module is fully usable.
	*/
	virtual void BuildVideoDriver(const video::DriverConfig& config) = 0;

	//! Creates the scene manager component of the engine
	/**
	Requires video driver and the input system.
	After this call all scene modules are fully usable
	*/
	virtual void BuildSceneManager() = 0;
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

	//! Closes the associated window.
	virtual void CloseDevice() = 0;

	virtual bool HandleSystemMessages() = 0;

	//! Runs all engine internal actions.
	/**
	\param [out] numSecsPassed The time in seconds since the last call to Run.
	\return Returns true if the device is still running, returns false once when the device was closed.
	*/
	virtual bool Run(float& numSecsPassed) = 0;

	virtual double GetTime() const = 0;

	virtual StrongRef<video::VideoDriver> GetDriver() const = 0;
	virtual StrongRef<scene::SceneManager> GetSceneManager() const = 0;
	virtual StrongRef<input::InputSystem> GetInputSystem() const = 0;
	virtual StrongRef<video::MaterialLibrary> GetMaterialLibrary() const = 0;
	virtual StrongRef<video::ImageSystem> GetImageSystem() const = 0;
	virtual StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const = 0;
	virtual StrongRef<gui::Window> GetWindow() const = 0;
	virtual StrongRef<video::MeshSystem> GetMeshSystem() const = 0;
};

//! Create a new lux device.
LUX_API StrongRef<LuxDevice> CreateDevice();

}    //namespace lux

#endif
