#ifdef LUX_WINDOWS
#include "LuxDeviceWin32.h"
#include "DllMainWin32.h"

#include "core/Clock.h"
#include "core/Logger.h"
#include "core/StringConverter.h"
#include "core/ReferableFactory.h"

#include "resources/ResourceSystem.h"

#include "scene/SceneManagerImpl.h"
#include "scene/particle/ParticleSystemManager.h"

#include "io/FileSystem.h"

#include "gui/CursorControlWin32.h"
#include "gui/GUIEnvironmentImpl.h"

#include "core/lxUnicodeConversion.h"

#include "core/ReferableRegister.h"

#include "video/mesh/MeshSystem.h"
#include "video/MaterialLibrary.h"

#include "video/images/ImageSystem.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/RendererD3D9.h"
#include "video/d3d9/AdapterInformationD3D9.h"
#endif

#include "input/InputSystem.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/raw_input/RawInputReceiver.h"
#endif

#include <WinUser.h>

#include "Win32Exception.h"

namespace lux
{

LUX_API StrongRef<LuxDevice> CreateDevice()
{
	return LUX_NEW(LuxDeviceWin32);
}

LuxDeviceWin32::LuxDeviceWin32()
{
	// If there are logs which aren't written, write them to the default file.
	if(log::EngineLog.HasUnsetLogs())
		log::EngineLog.SetNewPrinter(log::FilePrinter, true);
	if(!log::FilePrinter->IsInit())
		log::FilePrinter->Init();

	// Create the singleton classes
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

	log::Info("Lux core was build.");
}

LuxDeviceWin32::~LuxDeviceWin32()
{
	m_GUIEnv.Reset();
	m_SceneManager.Reset();

	scene::ParticleSystemManager::Destroy();

#ifdef LUX_COMPILE_WITH_D3D9
	video::VideoDriverD3D9* driver = dynamic_cast<video::VideoDriverD3D9*>(video::VideoDriver::Instance());

	// Free all shared resources, default materials, invalid materials and so on
	if(driver)
		driver->CleanUp();
#endif

	video::MeshSystem::Destroy();
	video::ImageSystem::Destroy();
	video::MaterialLibrary::Destroy();
	video::VideoDriver::Destroy();

	core::ResourceSystem::Destroy();
	core::ReferableFactory::Destroy();
	io::FileSystem::Destroy();

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver.Reset();
#endif

	input::InputSystem::Destroy();

	m_Window.Reset();

	log::Info("Shutdown complete.");
}

void LuxDeviceWin32::BuildWindow(u32 width, u32 height, const String& title)
{
	if(m_Window) {
		log::Warning("Window already built.");
		return;
	}

	log::Info("Create new Lux window \"~s\".", title);

	m_Window = LUX_NEW(gui::WindowWin32)(math::Dimension2U(width, height), title, this);
}

void LuxDeviceWin32::BuildInputSystem(bool isForeground)
{
	// If system already build -> no op
	if(input::InputSystem::Instance()) {
		log::Warning("Input system alread built.");
		return;
	}

	if(!m_Window->GetDeviceWindow())
		throw core::ErrorException("Missing window");

	input::InputSystem::Initialize();
	auto inputSys = input::InputSystem::Instance();
	inputSys->SetDefaultForegroundHandling(isForeground);
	inputSys->SetForegroundState(m_Window->IsFocused());

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver = LUX_NEW(input::RawInputReceiver)(inputSys, (HWND)m_Window->GetDeviceWindow());
	u32 keyboardCount = m_RawInputReceiver->DiscoverDevices(input::EEventSource::Keyboard);
	if(keyboardCount == 0)
		throw core::RuntimeException("No keyboard found on system.");
#else
	throw core::NotImplementedException();
#endif

	log::Info("Built Input System.");
}

void LuxDeviceWin32::BuildVideoDriver(const video::DriverConfig& config)
{
	// If system already build -> no op
	if(video::VideoDriver::Instance()) {
		log::Warning("Videodriver already build.");
		return;
	}

	log::Info("Building Video Driver.");

	StrongRef<video::VideoDriver> driver;
	if(!config.adapter || config.adapter->GetDriverType() == video::EDriverType::Null)
		return;

#ifdef LUX_COMPILE_WITH_D3D9
	if(config.adapter->GetDriverType() == video::EDriverType::Direct3D9)
		driver = LUX_NEW(video::VideoDriverD3D9)(config, m_Window);
#endif

	if(!driver)
		throw core::NotImplementedException();

	video::VideoDriver::Initialize(driver);

	video::MaterialLibrary::Initialize();
	auto invalidMaterial = video::MaterialLibrary::Instance()->CreateMaterial("debugOverlay");
	invalidMaterial->SetDiffuse(video::Color(255, 0, 255));
	driver->GetRenderer()->SetInvalidMaterial(invalidMaterial);

	BuildImageSystem();

	video::MeshSystem::Initialize();
}

core::Array<video::EDriverType> LuxDeviceWin32::GetDriverTypes()
{
	core::Array<video::EDriverType> out;
#ifdef LUX_COMPILE_WITH_D3D9
	out.PushBack(video::EDriverType::Direct3D9);
#endif
	return out;
}

StrongRef<video::AdapterList> LuxDeviceWin32::GetVideoAdapters(video::EDriverType driver)
{
#ifdef LUX_COMPILE_WITH_D3D9
	if(driver == video::EDriverType::Direct3D9)
		return LUX_NEW(video::AdapterListD3D9);
#endif
	throw core::NotImplementedException();
}

void LuxDeviceWin32::BuildSceneManager()
{
	// If system already build -> no op
	if(m_SceneManager) {
		log::Warning("Scene Manager already built.");
		return;
	}

	if(!scene::ParticleSystemManager::Instance())
		scene::ParticleSystemManager::Initialize();

	log::Info("Build Scene Manager.");
	m_SceneManager = LUX_NEW(scene::SceneManagerImpl);
}

void LuxDeviceWin32::BuildImageSystem()
{
	if(video::ImageSystem::Instance()) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Build Image System.");
	video::ImageSystem::Initialize();
}

void LuxDeviceWin32::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Build GUI Environment.");
	m_GUIEnv = LUX_NEW(gui::GUIEnvironmentImpl);
}

void LuxDeviceWin32::BuildAll(const video::DriverConfig& config)
{
	BuildWindow(config.display.width, config.display.height, "Window");
	BuildInputSystem();
	BuildVideoDriver(config);
	BuildSceneManager();
	BuildGUIEnvironment();
}

bool LuxDeviceWin32::Run(float& numSecsPassed)
{
	static auto startTime = core::Clock::GetTicks();
	double time;

	auto endTime = core::Clock::GetTicks();
	if(endTime == startTime)
		time = 0.000001;
	else
		time = (double)(endTime - startTime) * 0.001;

	numSecsPassed = (float)time;

	startTime = endTime;

	bool wasQuit = false;
	if(m_Window) {
		auto windowWin32 = m_Window.As<gui::WindowWin32>();
		if(windowWin32->RunMessageQueue()) {
			m_Window->Close();
			wasQuit = true;
			m_Window = nullptr;
		}
	}

	return !wasQuit;
}

bool LuxDeviceWin32::OnMsg(
	UINT uiMessage,
	WPARAM WParam,
	LPARAM LParam,
	LRESULT& result)
{
	result = 0;
#ifdef LUX_COMPILE_WITH_RAW_INPUT
	if(m_RawInputReceiver && m_RawInputReceiver->HandleMessage(uiMessage, WParam, LParam, result))
		return true;
#endif

	switch(uiMessage) {
	case WM_ERASEBKGND:
		return true;

	case WM_SYSCOMMAND:
		if((WParam & 0xFFF0) == SC_SCREENSAVE ||
			(WParam & 0xFFF0) == SC_MONITORPOWER)
			return true;
		break;
	}

	return false;
}

} // namespace lux

#endif // LUX_WINDOWS
