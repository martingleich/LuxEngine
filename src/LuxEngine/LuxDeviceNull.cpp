#include "LuxDeviceNull.h"
#include "core/Clock.h"
#include "core/Logger.h"

#include "core/ModuleFactory.h"
#include "core/ReferableFactory.h"
#include "core/ResourceSystem.h"

#include "io/FileSystem.h"

#include "video/mesh/MeshSystem.h"
#include "video/MaterialLibrary.h"

#include "video/images/ImageSystem.h"

#include "video/VideoDriver.h"
#include "video/RenderTarget.h"
#include "video/Renderer.h"
#include "video/Canvas3D.h"

#include "scene/Scene.h"
#include "scene/SceneRenderer.h"

#include "gui/GUIEnvironment.h"
#include "gui/Window.h"
#include "gui/Cursor.h"

namespace lux
{
LuxDeviceNull::LuxDeviceNull()
{
	// If there are logs which aren't written, write them to the default file.
	if(log::EngineLog.HasUnsetLogs())
		log::EngineLog.SetPrinter(log::FilePrinter, true);

	// Create the singleton classes
	core::ModuleFactory::Initialize();
	io::FileSystem::Initialize();
	core::ReferableFactory::Initialize();
	core::ResourceSystem::Initialize();

	auto resourceSystem = core::ResourceSystem::Instance();

	log::Log("Starting time ~a", core::Clock::GetDateAndTime());

	// Register resource types
	resourceSystem->AddType(core::ResourceType::Mesh);
	resourceSystem->AddType(core::ResourceType::Image);
	resourceSystem->AddType(core::ResourceType::ImageList);
	resourceSystem->AddType(core::ResourceType::Texture);
	resourceSystem->AddType(core::ResourceType::CubeTexture);
	resourceSystem->AddType(core::ResourceType::Font);
	resourceSystem->AddType(core::ResourceType::Sound);

	resourceSystem->SetCaching(core::ResourceType::ImageList, false);
	resourceSystem->SetCaching(core::ResourceType::Image, false);

	// Register all referable object registers with LUX_REGISTER_REFERABLE_CLASS
	lux::core::impl_referableRegister::RunAllRegisterReferableFunctions();
	lux::core::impl_moduleRegister::RunAllModuleFactoryBlocks();

	log::Info("Lux core was built.");
}

LuxDeviceNull::~LuxDeviceNull()
{
}

void LuxDeviceNull::ReleaseModules()
{
	m_GUIEnv.Reset();
	m_Scene.Reset();
	m_SceneRenderer.Reset();

	video::MeshSystem::Destroy();
	video::ImageSystem::Destroy();
	video::MaterialLibrary::Destroy();
	core::ResourceSystem::Destroy();

	video::Canvas3DSystem::Destroy();
	video::VideoDriver::Destroy();

	io::FileSystem::Destroy();
	core::ReferableFactory::Destroy();
	core::ModuleFactory::Destroy();
}

void LuxDeviceNull::BuildVideoDriver(const video::DriverConfig& config, void* user)
{
	// If system already build -> no op
	if(video::VideoDriver::Instance()) {
		log::Warning("Videodriver already built.");
		return;
	}

	log::Info("Building Video Driver.");

	if(!config.adapter)
		return;

	video::VideoDriverInitData init;
	init.config = config;
	init.destHandle = GetWindow()->GetDeviceWindow();
	init.user = user;
	auto driver = core::ModuleFactory::Instance()->CreateModule(
		"VideoDriver", config.adapter->GetDriverType(), init).AsStrong<video::VideoDriver>();

	video::VideoDriver::Initialize(driver);

	video::MaterialLibrary::Initialize();

	BuildImageSystem();

	video::MeshSystem::Initialize();
	video::Canvas3DSystem::Initialize();
}

core::Array<core::String> LuxDeviceNull::GetDriverTypes()
{
	return core::ModuleFactory::Instance()->GetModuleFactories("VideoDriver");
}

StrongRef<video::AdapterList> LuxDeviceNull::GetVideoAdapters(const core::String& driver)
{
	return core::ModuleFactory::Instance()->CreateModule("AdapterList", driver, core::ModuleInitData()).As<video::AdapterList>();
}

void LuxDeviceNull::BuildScene(const core::String& name, void* user)
{
	// If system already build -> no op
	if(m_Scene) {
		log::Warning("Scene already built.");
		return;
	}

	log::Info("Building Scene.");
	m_Scene = CreateScene();
	m_SceneRenderer = CreateSceneRenderer(name.IsEmpty() ? "SimpleForward" : name, m_Scene, user);
}

StrongRef<scene::Scene> LuxDeviceNull::CreateScene()
{
	return LUX_NEW(scene::Scene);
}

StrongRef<scene::SceneRenderer> LuxDeviceNull::CreateSceneRenderer(const core::String& name, scene::Scene* scene, void* user)
{
	scene::SceneRendererInitData init;
	init.scene = scene;
	init.user = user;
	return core::ModuleFactory::Instance()->CreateModule("SceneRenderer", name, init).As<scene::SceneRenderer>();
}

void LuxDeviceNull::BuildImageSystem()
{
	if(video::ImageSystem::Instance()) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Building ImageSystem.");
	video::ImageSystem::Initialize();
}

void LuxDeviceNull::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Building GUI-Environment.");
	m_GUIEnv = LUX_NEW(gui::GUIEnvironment)(GetWindow(), GetCursor());
}

void LuxDeviceNull::BuildAll(const video::DriverConfig& config)
{
	BuildWindow(config.display.width, config.display.height, "Window");
	BuildInputSystem();
	BuildVideoDriver(config);
	BuildScene("SimpleForward");
	BuildGUIEnvironment();
}

StrongRef<scene::Scene> LuxDeviceNull::GetScene() const
{
	return m_Scene;
}
StrongRef<scene::SceneRenderer> LuxDeviceNull::GetSceneRenderer() const
{
	return m_SceneRenderer;
}
StrongRef<gui::GUIEnvironment> LuxDeviceNull::GetGUIEnvironment() const
{
	return m_GUIEnv;
}

namespace
{
class DefaultSimpleFrameLoop
{
public:
	DefaultSimpleFrameLoop(LuxDevice* device, const LuxDevice::SimpleFrameLoop& loop) :
		m_FrameLoop(loop),
		m_Device(device)
	{
		m_Window = m_Device->GetWindow();
		m_GUIEnv = m_Device->GetGUIEnvironment();
		m_Scene = m_Device->GetScene();
		m_SceneRenderer = m_Device->GetSceneRenderer();
		m_Driver = video::VideoDriver::Instance();
		m_Renderer = m_Driver->GetRenderer();

		m_PerformDriverReset = false;
	}

	bool CallPreFrame()
	{
		bool ret = m_FrameLoop.useDefaultPreFrame ? DefaultPreFrame() : true;
		if(ret && m_FrameLoop.callback)
			return m_FrameLoop.callback->PreFrame();
		else
			return ret;
	}

	void CallDoFrame(float secsPassed)
	{
		DefaultDoFrame(secsPassed);
	}

private:
	bool DefaultPreFrame()
	{
		if(!m_Window || !m_Renderer || !m_Driver)
			return false;

		if(m_PerformDriverReset) {
			m_PerformDriverReset = !ResetDriver();
			if(m_FrameLoop.callback)
				m_FrameLoop.callback->DriverReset(!m_PerformDriverReset);

			// Always abort the frame, event if the reset was successfull
			// Since the frame took much more time than secsPassed.
			return false;
		}

		// Wait until we are allowed to do something
		bool wait;
		bool pausing = false;
		do {
			wait = false;
			if(m_FrameLoop.pauseOnLostFocus && !m_Window->IsActive())
				wait = true;
			if(m_FrameLoop.pauseOnMinimize && m_Window->IsMinimized())
				wait = true;

			if(wait) {
				log::Debug("Pause frame loop");
				pausing = true;
			}

			// Wait for change, abort if quit
			if(wait && !m_Device->WaitForWindowChange())
				return false;
		} while(wait);

		if(pausing) {
			log::Debug("Resume frame loop");
			return false;
		} else {
			return true;
		}
	}

	void DefaultDoFrame(float secsPassed)
	{
		video::RenderStatistics::Instance()->EndFrame();
		video::RenderStatistics::Instance()->BeginFrame();

		if(m_Scene)
			m_Scene->AnimateAll(secsPassed);
		if(m_GUIEnv)
			m_GUIEnv->Update(secsPassed);

		if(!m_Renderer->Present()) {
			log::Error("Present failed");
			auto state = m_Driver->GetDeviceState();
			if(state != video::EDeviceState::OK) {
				log::Debug("Trying to reset driver.");
				m_PerformDriverReset = true;
			}
		}

		if(m_FrameLoop.callback)
			m_FrameLoop.callback->PostMove(secsPassed);

		if(m_SceneRenderer) {
			m_SceneRenderer->DrawScene(true, false);
		} else {
			m_Renderer->Clear(true, true, true);
			m_Renderer->BeginScene();
		}
		if(m_FrameLoop.callback)
			m_FrameLoop.callback->PostSceneRender(secsPassed);

		if(!m_Renderer->GetRenderTarget().IsBackbuffer()) {
			// If the scene graph didn't render into the backbuffer,
			// finish the scene and renderer the gui into the backbuffer.
			m_Renderer->EndScene();
			m_Renderer->BeginScene();
			m_Renderer->SetRenderTarget(nullptr);
			m_Renderer->Clear(true, true, true);
		}

		if(m_GUIEnv)
			m_GUIEnv->Render();

		if(m_FrameLoop.callback) {
			m_FrameLoop.callback->PostGUIRender(secsPassed);
			m_FrameLoop.callback->PostFrameRender(secsPassed);
		}

		m_Renderer->EndScene();

		if(m_FrameLoop.callback)
			m_FrameLoop.callback->PostFrame(secsPassed);
	}

	bool ResetDriver()
	{
		auto state = m_Driver->GetDeviceState();
		if(state == video::EDeviceState::Error) {
			log::Error("Internal driver error");
			m_Window->Close();
			return false;
		}

		// Wait until the window is active
		while(!m_Window->IsActive() && m_Device->WaitForWindowChange());

		state = m_Driver->GetDeviceState();
		if(state == video::EDeviceState::Error) {
			log::Error("Internal driver error");
			m_Window->Close();
			return false;
		} else if(state == video::EDeviceState::OK) {
			return true;
		} else if(state == video::EDeviceState::NotReset) {
			if(!m_Driver->Reset(m_Driver->GetConfig())) {
				log::Error("Can't restore driver");
				m_Window->Close();
				return false;
			} else {
				log::Info("Performed driver reset");
				return true;
			}
		} else {
			m_Device->Sleep(100);
		}

		return false;
	}

private:
	LuxDevice::SimpleFrameLoop m_FrameLoop;
	bool m_PerformDriverReset;

	LuxDevice* m_Device;
	gui::Window* m_Window;
	gui::GUIEnvironment* m_GUIEnv;
	scene::Scene* m_Scene;
	scene::SceneRenderer* m_SceneRenderer;
	video::VideoDriver* m_Driver;
	video::Renderer* m_Renderer;
};
}

void LuxDeviceNull::RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop)
{
	DefaultSimpleFrameLoop defLoop(this, frameLoop);

	u64 startTime;
	u64 endTime;
	startTime = core::Clock::GetTicks();
	float invTicksPerSecond = 1.0f / core::Clock::TicksPerSecond();
	float secsPassed = 0;
	float minSecsPassed = 1.0f / frameLoop.maxFrameRate;
	u32 runWaitTime = 1;
	while(Run(runWaitTime)) {
		if(!defLoop.CallPreFrame()) {
			startTime = core::Clock::GetTicks();
			continue;
		}

		if(secsPassed > minSecsPassed)
			defLoop.CallDoFrame(secsPassed);

		endTime = core::Clock::GetTicks();
		secsPassed = (endTime - startTime)*invTicksPerSecond;
		if(secsPassed > minSecsPassed)
			startTime = endTime;
	}
}

} // namespace lux