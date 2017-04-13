#ifndef INCLUDED_LUXDEVICE_H
#define INCLUDED_LUXDEVICE_H
#include "core/lxString.h"
#include "input/EventReceiver.h"

namespace lux
{

namespace core
{
class ReferableFactory;
class ResourceSystem;
class Timer;
}

namespace io
{
class FileSystem;
}

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
}

namespace scene
{
class SceneManager;
class MeshSystem;
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

	//! Initialize the core engine functionality
	/**
	Must not be called by the user.
	Automatically called when creating a new device.
	After this call the fileSystem, logging, referable, and timing modules are usable
	*/
	virtual bool BuildCoreDevice() = 0;

	//! Create the window associated with the engine.
	/**
	Requires core device.
	\param width The width of the window.
	\param height The height of the window.
	\param title The title of the window.
	*/
	virtual bool BuildWindow(u32 Width, u32 height, const string& title) = 0;
	virtual void SetOwnWindow(void* OwnWindow) = 0;

	//! Create the input system of the engine.
	/**
	Requires associated window.
	Initialized the input system of the engine.
	*/
	virtual bool BuildInputSystem() = 0;

	//! Creates the video driver component of the engine.
	/**
	Requires associated window.
	After this call all video modules are fully usable.
	The image module is fully usable.
	*/
	virtual bool BuildVideoDriver(const video::DriverConfig& config) = 0;

	//! Creates the scene manager component of the engine
	/**
	Requires video driver and the input system.
	After this call all scene modules are fully usable
	*/
	virtual bool BuildSceneManager() = 0;
	virtual bool BuildGUIEnvironment() = 0;

	//! Build the image system of the engine.
	/**
	Requires the core device.
	Is automatic called by BuildVideoDriver, but can also
	be called on it's own.
	*/
	virtual bool BuildImageSystem() = 0;

	//! Build all components.
	/**
	The created window has the same resolution as the backbuffer.
	*/
	virtual bool BuildAll(const video::DriverConfig& config) = 0;

	//! Closes the associated window.
	virtual void CloseDevice() = 0;

	virtual bool HandleSystemMessages() = 0;
	virtual void SetUserEventReceiver(input::UserEventReceiver* Receiver) = 0;
	virtual StrongRef<input::UserEventReceiver> GetUserEventReceiver() const = 0;
	virtual void PostEvent(const input::Event& event, input::EEventTarget target = input::EEventTarget::All) = 0;

	//! Runs all engine internal actions.
	/**
	i.e. Timer, window message handling, etc.
	\param [out] numSecsPassed The time in seconds since the last call to Run.
	\return Returns true if the device is still running, returns false once when the device was closed.
	*/
	virtual bool Run(float& numSecsPassed) = 0;

	virtual double GetTime() const = 0;

	virtual StrongRef<video::VideoDriver> GetDriver() const = 0;
	virtual StrongRef<scene::SceneManager> GetSceneManager() const = 0;
	virtual StrongRef<input::InputSystem> GetInputSystem() const = 0;
	virtual StrongRef<video::MaterialLibrary> GetMaterialLibrary() const = 0;
	virtual StrongRef<io::FileSystem> GetFileSystem() const = 0;
	virtual StrongRef<video::ImageSystem> GetImageSystem() const = 0;
	virtual StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const = 0;
	virtual StrongRef<core::ReferableFactory> GetReferableFactory() const = 0;
	virtual StrongRef<core::Timer> GetTimer() const = 0;
	virtual StrongRef<gui::Window> GetWindow() const = 0;
	virtual StrongRef<core::ResourceSystem> GetResourceSystem() const = 0;
	virtual StrongRef<scene::MeshSystem> GetMeshSystem() const = 0;
};

//! Create a new lux device.
LUX_API StrongRef<LuxDevice> CreateDevice();

}    //namespace lux

#endif
