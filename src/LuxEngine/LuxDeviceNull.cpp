#include "LuxDeviceNull.h"
#include "core/Clock.h"
#include "core/Logger.h"

#include "core/ReferableRegister.h"
#include "core/ModuleFactoryRegister.h"
#include "core/ReferableFactory.h"
#include "resources/ResourceSystem.h"

#include "io/FileSystem.h"

#include "video/mesh/MeshSystem.h"
#include "video/MaterialLibrary.h"

#include "video/images/ImageSystem.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"

#include "scene/Scene.h"
#include "scene/SceneRenderer.h"
#include "scene/particle/ParticleSystemManager.h"

#include "gui/GUIEnvironment.h"
#include "gui/Window.h"

namespace lux
{
LuxDeviceNull::LuxDeviceNull()
{
	// If there are logs which aren't written, write them to the default file.
	if(log::EngineLog.HasUnsetLogs())
		log::EngineLog.SetNewPrinter(log::FilePrinter, true);
	if(!log::FilePrinter->IsInit())
		log::FilePrinter->Init();

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

	log::Info("Lux core was build.");
}

LuxDeviceNull::~LuxDeviceNull()
{
}

void LuxDeviceNull::ReleaseModules()
{
	m_GUIEnv.Reset();
	m_Scene.Reset();
	m_SceneRenderer.Reset();
	scene::ParticleSystemManager::Destroy();

	auto driver = video::VideoDriver::Instance();
	if(driver)
		driver->ReleaseSharedData();

	video::MeshSystem::Destroy();
	video::ImageSystem::Destroy();
	video::MaterialLibrary::Destroy();
	core::ResourceSystem::Destroy();

	video::VideoDriver::Destroy();

	io::FileSystem::Destroy();
	core::ReferableFactory::Destroy();
	core::ModuleFactory::Destroy();
}

void LuxDeviceNull::BuildVideoDriver(const video::DriverConfig& config, void* user)
{
	// If system already build -> no op
	if(video::VideoDriver::Instance()) {
		log::Warning("Videodriver already build.");
		return;
	}

	log::Info("Building Video Driver.");

	if(!config.adapter)
		return;

	video::VideoDriverInitData init;
	init.config = config;
	init.window = GetWindow();
	init.user = user;
	StrongRef<video::VideoDriver> driver = core::ModuleFactory::Instance()->CreateModule(
		"VideoDriver", config.adapter->GetDriverType(), init);

	video::VideoDriver::Initialize(driver);

	video::MaterialLibrary::Initialize();
	auto invalidMaterial = video::MaterialLibrary::Instance()->CreateMaterial("debugOverlay");
	invalidMaterial->SetDiffuse(video::Color(255, 0, 255));
	driver->GetRenderer()->SetInvalidMaterial(invalidMaterial);

	BuildImageSystem();

	video::MeshSystem::Initialize();
}

core::Array<String> LuxDeviceNull::GetDriverTypes()
{
	return core::ModuleFactory::Instance()->GetModuleFactories("VideoDriver");
}

StrongRef<video::AdapterList> LuxDeviceNull::GetVideoAdapters(const String& driver)
{
	return core::ModuleFactory::Instance()->CreateModule("AdapterList", driver, core::ModuleInitData());
}

void LuxDeviceNull::BuildScene(const String& name, void* user)
{
	// If system already build -> no op
	if(m_Scene) {
		log::Warning("Scene already built.");
		return;
	}

	if(!scene::ParticleSystemManager::Instance())
		scene::ParticleSystemManager::Initialize();

	log::Info("Build Scene.");
	m_Scene = CreateScene();
	m_SceneRenderer = CreateSceneRenderer(name.IsEmpty() ? "SimpleForward" : name, user);
}

StrongRef<scene::Scene> LuxDeviceNull::CreateScene()
{
	return LUX_NEW(scene::Scene);
}

StrongRef<scene::SceneRenderer> LuxDeviceNull::CreateSceneRenderer(const String& name, void* user)
{
	scene::SceneRendererInitData init;
	init.user = user;
	return core::ModuleFactory::Instance()->CreateModule("SceneRenderer", name, init);
}

void LuxDeviceNull::BuildImageSystem()
{
	if(video::ImageSystem::Instance()) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Build Image System.");
	video::ImageSystem::Initialize();
}

void LuxDeviceNull::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Build GUI Environment.");
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

class DefaultSimpleFrameLoop
{
public:
	DefaultSimpleFrameLoop(LuxDevice* device) :
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

	bool CallPreFrame(const LuxDevice::SimpleFrameLoop& user)
	{
		if(user.preFrameProc)
			return user.preFrameProc(user.userData);
		else
			return DefaultPreFrame(user);
	}

	void CallDoFrame(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		if(user.frameProc)
			user.frameProc(secsPassed, user.userData);
		else
			DefaultDoFrame(secsPassed, user);
	}

	void CallPostMove(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		if(user.postMoveProc)
			user.postMoveProc(secsPassed, user.userData);
		else
			DefaultPostMove(secsPassed, user);
	}

	void CallPostRender(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		if(user.postRenderProc)
			user.postRenderProc(secsPassed, user.userData);
		else
			DefaultPostRender(secsPassed, user);
	}

private:
	bool DefaultPreFrame(const LuxDevice::SimpleFrameLoop& user)
	{
		if(!m_Window || !m_Renderer || !m_Driver)
			return false;

		if(m_PerformDriverReset) {
			m_PerformDriverReset = !ResetDriver();
			// Always abort the frame, event if the reset was successfull
			// Since the frame took much more time than secsPassed.
			return false;
		}

		// Wait unti we are allowed to do something
		bool wait;
		bool pausing = false;
		do {
			wait = false;
			if(user.pauseOnLostFocus && !m_Window->IsActive())
				wait = true;
			if(user.pauseOnMinimize && m_Window->IsMinimized())
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

	void DefaultDoFrame(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		video::RenderStatistics::Instance()->EndFrame();
		video::RenderStatistics::Instance()->BeginFrame();

		secsPassed *= user.timeScale;

		if(m_Scene)
			m_Scene->AnimateAll(secsPassed);
		if(m_GUIEnv)
			m_GUIEnv->Update(secsPassed);

		CallPostMove(secsPassed, user);

		if(m_SceneRenderer)
			m_SceneRenderer->DrawScene(m_Scene, true, false);
		else {
			m_Renderer->Clear(true, true, true);
			m_Renderer->BeginScene();
		}
		if(m_GUIEnv)
			m_GUIEnv->Render();

		CallPostRender(secsPassed, user);

		m_Renderer->EndScene();

		if(!m_Renderer->Present()) {
			log::Error("Present failed");
			auto state = m_Driver->GetDeviceState();
			if(state != video::EDeviceState::OK) {
				log::Debug("Trying to reset driver.");
				m_PerformDriverReset = true;
			}
		}
	}

	void DefaultPostMove(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		LUX_UNUSED(secsPassed);
		LUX_UNUSED(user);
	}

	void DefaultPostRender(float secsPassed, const LuxDevice::SimpleFrameLoop& user)
	{
		LUX_UNUSED(secsPassed);
		LUX_UNUSED(user);
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
	bool m_PerformDriverReset;

	LuxDevice* m_Device;
	gui::Window* m_Window;
	gui::GUIEnvironment* m_GUIEnv;
	scene::Scene* m_Scene;
	scene::SceneRenderer* m_SceneRenderer;
	video::VideoDriver* m_Driver;
	video::Renderer* m_Renderer;
};

void LuxDeviceNull::RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop)
{
	DefaultSimpleFrameLoop defLoop(this);

	u64 startTime;
	u64 endTime;
	startTime = core::Clock::GetTicks();
	float invTicksPerSecond = 1.0f / core::Clock::TicksPerSecond();
	float secsPassed = 0.0f;
	while(Run()) {
		if(!defLoop.CallPreFrame(frameLoop)) {
			startTime = core::Clock::GetTicks();
			continue;
		}

		if(secsPassed != 0)
			defLoop.CallDoFrame(secsPassed, frameLoop);

		endTime = core::Clock::GetTicks();
		secsPassed = (endTime - startTime)*invTicksPerSecond;
		if(secsPassed != 0)
			startTime = endTime;
	}
}

} // namespace lux