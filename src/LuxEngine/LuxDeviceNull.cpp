#include "LuxDeviceNull.h"
#include "core/Clock.h"
#include "core/Logger.h"

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

#include "video/mesh/MeshLoaderOBJ.h"
#include "video/mesh/MeshLoaderX.h"

#include "video/images/ImageLoaderBMP.h"
#include "video/images/ImageLoaderPNM.h"
#include "video/images/ImageLoaderTGA.h"
#include "video/images/ImageLoaderPNG.h"

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
#include "video/images/ImageLoaderD3DX.h"
#endif

#include "video/images/ImageWriterBMP.h"
#include "video/images/ImageWriterTGA.h"

#include "gui/GUIEnvironment.h"
#include "gui/Window.h"
#include "gui/Cursor.h"

namespace lux
{
LuxDeviceNull::LuxDeviceNull()
{
	// If there are logs which aren't written, write them to the default file.
	if(!log::GetPrinter())
		log::SetPrinter(log::FilePrinter);

	// Create the singleton classes
	io::FileSystem::Initialize();
	core::ReferableFactory::Initialize();
	core::ResourceSystem::Initialize();

	auto resourceSystem = core::ResourceSystem::Instance();

	log::Log("Starting time {}", core::Clock::GetDateAndTime());

	// Register resource types
	resourceSystem->AddType(core::ResourceType::Animation);
	resourceSystem->AddType(core::ResourceType::CubeTexture);
	resourceSystem->AddType(core::ResourceType::Font);
	resourceSystem->AddType(core::ResourceType::Image);
	resourceSystem->AddType(core::ResourceType::Mesh);
	resourceSystem->AddType(core::ResourceType::Sound);
	resourceSystem->AddType(core::ResourceType::Texture);

	resourceSystem->SetCaching(core::ResourceType::Image, false);

	// Register all referable object registers with LUX_REGISTER_REFERABLE_CLASS
	lux::core::impl_referableRegister::RunAllRegisterReferableFunctions();

	log::Info("Lux core was built.");
}

LuxDeviceNull::~LuxDeviceNull()
{
}

void LuxDeviceNull::ReleaseModules()
{
	gui::GUIEnvironment::Destroy();

	video::MeshSystem::Destroy();
	video::ImageSystem::Destroy();

	video::MaterialLibrary::Destroy();
	core::ResourceSystem::Destroy();

	video::Canvas3DSystem::Destroy();
	video::ShaderFactory::Destroy();
	video::VideoDriver::Destroy();

	io::FileSystem::Destroy();
	core::ReferableFactory::Destroy();
}

void LuxDeviceNull::BuildVideoDriver(const video::DriverConfig& config)
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
	auto driverEntry = m_VideoDrivers.Get(config.adapter->GetDriverType());
	auto driver = driverEntry.driverCreateFunc(init);
	if(!driver)
		throw core::NotImplementedException(config.adapter->GetDriverType().AsView());

	video::VideoDriver::Initialize(driver);

}

void LuxDeviceNull::BuildVideoDriverHelpers()
{
	if(!video::ShaderFactory::Instance())
		video::ShaderFactory::Initialize();

	if(!video::Canvas3DSystem::Instance())
		video::Canvas3DSystem::Initialize();
}

void LuxDeviceNull::BuildMaterialLibrary()
{
	if(video::MaterialLibrary::Instance())
		return;
	video::MaterialLibrary::Initialize();
}

void LuxDeviceNull::BuildMeshSystem(video::Material* defaultMaterial)
{
	if(video::MeshSystem::Instance())
		return;

	if(!defaultMaterial)
		defaultMaterial = video::MaterialLibrary::Instance()->GetMaterial(video::MaterialLibrary::EKnownMaterial::Solid);
	video::MeshSystem::Initialize();

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(video::MeshLoaderOBJ));
	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(video::MeshLoaderX));
}

void LuxDeviceNull::BuildImageSystem()
{
	if(video::ImageSystem::Instance())
		return;

	video::ImageSystem::Initialize();

	auto resSys = core::ResourceSystem::Instance();
#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
	auto driver = video::VideoDriver::Instance();
	if(driver && driver->GetVideoDriverType() == video::DriverType::Direct3D9) {
		IDirect3DDevice9* d3dDevice = reinterpret_cast<IDirect3DDevice9*>(driver->GetLowLevelDevice());
		resSys->AddResourceLoader(LUX_NEW(video::ImageLoaderD3DX)(d3dDevice));
	}
#endif // LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

	resSys->AddResourceLoader(LUX_NEW(video::ImageLoaderBMP));
	resSys->AddResourceLoader(LUX_NEW(video::ImageLoaderPNM));
	resSys->AddResourceLoader(LUX_NEW(video::ImageLoaderTGA));
	resSys->AddResourceLoader(LUX_NEW(video::ImageLoaderPNG));

	resSys->AddResourceWriter(LUX_NEW(video::ImageWriterBMP));
	resSys->AddResourceWriter(LUX_NEW(video::ImageWriterTGA));
}

void LuxDeviceNull::BuildGUIEnvironment()
{
	if(gui::GUIEnvironment::Instance() != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Building GUI-Environment.");
	auto env = LUX_NEW(gui::GUIEnvironment)(GetWindow(), GetCursor());
	gui::GUIEnvironment::Initialize(env);
}

core::Array<core::Name> LuxDeviceNull::GetVideoDriverTypes()
{
	core::Array<core::Name> names;
	for(auto& v : m_VideoDrivers.Keys())
		names.PushBack(v);
	return names;
}

StrongRef<video::AdapterList> LuxDeviceNull::GetVideoAdapters(core::Name driver)
{
	auto driverEntry = m_VideoDrivers.Get(driver);
	return driverEntry.adapterListCreateFunc();
}

StrongRef<scene::Scene> LuxDeviceNull::CreateScene()
{
	return LUX_NEW(scene::Scene);
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
		m_GUIEnv = gui::GUIEnvironment::Instance();
		m_Scene = loop.scene;
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

		if(m_Scene) {
			m_Scene->DrawScene(true, false);
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
	video::VideoDriver* m_Driver;
	video::Renderer* m_Renderer;
};
}

void LuxDeviceNull::RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop)
{
	DefaultSimpleFrameLoop defLoop(this, frameLoop);

	core::Duration startTime;
	core::Duration endTime;
	core::Duration passedTime;
	startTime = core::Clock::GetTicks();
	float secsPassed = 0;
	float minSecsPassed = 1.0f / frameLoop.maxFrameRate;
	int runWaitTime = 1;
	while(Run(runWaitTime)) {
		if(!defLoop.CallPreFrame()) {
			startTime = core::Clock::GetTicks();
			continue;
		}

		if(secsPassed > minSecsPassed)
			defLoop.CallDoFrame(secsPassed);

		endTime = core::Clock::GetTicks();
		passedTime = (endTime - startTime);
		secsPassed = passedTime.AsSeconds();
		if(secsPassed > minSecsPassed)
			startTime = endTime;
	}
}

} // namespace lux