#ifndef INCLUDED_LUX_DEVICE_H
#define INCLUDED_LUX_DEVICE_H
#include "core/lxString.h"
#include "core/ReferenceCounted.h"
#include "video/DriverConfig.h"
#include "core/lxName.h"

namespace lux
{
namespace scene
{
class Scene;
class SceneRenderer;
class SceneRendererInitData;
}

namespace gui
{
class Window;
class Cursor;
class GUIEnvironment;
}
namespace core
{
class TimerManager;
}
namespace video
{
class VideoDriver;
class VideoDriverInitData;
class Material;
}
class LuxSystemInfo;

//! The main class for the engine
class LuxDevice : public ReferenceCounted
{
public:
	virtual ~LuxDevice() {}

	virtual core::Array<core::Name> GetVideoDriverTypes() = 0;
	virtual StrongRef<video::AdapterList> GetVideoAdapters(core::Name driver) = 0;

	void BuildAll(const video::DriverConfig& config)
	{
		BuildWindow(config.display.width, config.display.height, "Window");
		BuildInputSystem();
		BuildVideoDriver(config);
		BuildVideoDriverHelpers();
		BuildImageSystem();
		BuildMaterialLibrary();
		BuildMeshSystem();
		BuildGUIEnvironment();
	}
	virtual void BuildWindow(int width, int height, core::StringView title) = 0;
	virtual void BuildInputSystem(bool isForeground = true) = 0;
	virtual void BuildVideoDriver(const video::DriverConfig& config) = 0;

	// Canvas3DSystem, ShaderFactory
	virtual void BuildVideoDriverHelpers() = 0;

	virtual void BuildImageSystem() = 0;
	virtual void BuildMaterialLibrary() = 0;
	virtual void BuildMeshSystem(video::Material* defaultMaterial = nullptr) = 0;

	virtual void BuildGUIEnvironment() = 0;

	//! Creates a new scene.
	virtual StrongRef<scene::Scene> CreateScene() = 0;
	virtual StrongRef<scene::SceneRenderer> CreateSceneRenderer(core::Name renderer, scene::Scene* scene) = 0;

	///////////////////////////////////////////////////////////////////////////

	//! Runs all engine internal actions.
	/**
	\param waitTime Time this function waits for events to occur in milliseconds.
	\return Returns true if the device is still running, returns false once when the device was closed.
	*/
	virtual bool Run(int waitTime = 0) = 0;

	//! Wait until the window changed.
	/**
	May return at any time.
	Returns false if window change can't be detected, otherwise true.
	*/
	virtual bool WaitForWindowChange() = 0;

	class SimpleFrameLoopCallback
	{
	public:
		virtual ~SimpleFrameLoopCallback() {}

		virtual bool PreFrame() { return true; }
		virtual void PostMove(float secsPassed) { LUX_UNUSED(secsPassed); }
		virtual void PostSceneRender(float secsPassed) { LUX_UNUSED(secsPassed); }
		virtual void PostGUIRender(float secsPassed) { LUX_UNUSED(secsPassed); }
		virtual void PostFrameRender(float secsPassed) { LUX_UNUSED(secsPassed); }
		virtual void PostFrame(float secsPassed) { LUX_UNUSED(secsPassed); }

		virtual void DriverReset(bool succeded) { LUX_UNUSED(succeded); }
	};

	struct SimpleFrameLoop
	{
		float maxFrameRate = 1000.0f;
		bool pauseOnLostFocus = false;
		bool pauseOnMinimize = true;
		bool useDefaultPreFrame = true;
		SimpleFrameLoopCallback* callback = nullptr;

		scene::Scene* scene = nullptr;
		scene::SceneRenderer* sceneRenderer = nullptr;
	};

	//! Runs a simple frame loop.
	/**
	Performs a frame loop, call LuxDevice::Run, Renders and animates the gui and the scene, reset driver on failure.
	Pause the engine if the window is minimized and so on.
	\param loop Data for the frame loop.
	*/
	virtual void RunSimpleFrameLoop(const SimpleFrameLoop& loop) = 0;

	//! Pauses the calling process for some time
	virtual void Sleep(int millis) = 0;

	//! Get the os window of the engine.
	virtual StrongRef<gui::Window> GetWindow() const = 0;
	//! Get the os cursor of the engine.
	virtual StrongRef<gui::Cursor> GetCursor() const = 0;
	//! Retrieve system information.
	virtual StrongRef<LuxSystemInfo> GetSystemInfo() const = 0;

	virtual void AddSceneRenderer(core::Name name, scene::SceneRenderer* (*createFunc)(const scene::SceneRendererInitData&)) = 0;
};

//! Create a new lux device.
LUX_API StrongRef<LuxDevice> CreateDevice();

}    //namespace lux

#endif
