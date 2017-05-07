#ifdef LUX_WINDOWS
#include "LuxDeviceWin32.h"

#include "core/Clock.h"
#include "core/Logger.h"
#include "core/lxRandom.h"
#include "core/StringConverter.h"

#include "input/InputSystemImpl.h"
#include "scene/SceneManagerImpl.h"
#include "scene/mesh/MeshSystemImpl.h"
#include "resources/ResourceSystemImpl.h"
#include "video/MaterialLibraryImpl.h"
#include "core/ReferableFactoryImpl.h"

#include "io/FileSystemWin32.h"
#include "gui/WindowWin32.h"
#include "gui/CursorControlWin32.h"
#include "gui/GUIEnvironmentImpl.h"

#include "video/images/ImageSystemImpl.h"

#include "core/lxUnicodeConversion.h"

#include "core/ReferableRegister.h"

#include "video/VideoDriver.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/MaterialRendererD3D9.h"
#endif

#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/raw_input/RawInputReceiver.h"
#endif

#include <WinUser.h>
#include "LuxEngine/Win32Exception.h"

HMODULE g_Instance = NULL;
static const wchar_t WIN32_CLASS_NAME[] = L"Lux Window Class";

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD dwReasonForCall,
	LPVOID pvReserved)
{
	LUX_UNUSED(dwReasonForCall);
	LUX_UNUSED(pvReserved);

	g_Instance = hModule;

	return TRUE;
}

struct LogWin32Error
{
	DWORD error;
	LogWin32Error(DWORD e) : error(e)
	{
	}
};

lux::string GetWin32ErrorString(DWORD error)
{
	lux::string out;
	if(NOERROR != error) {
		const DWORD formatControl =
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM;

		LPWSTR textBuffer = NULL;
		DWORD count = FormatMessageW(formatControl,
			NULL,
			error,
			0,
			textBuffer,
			0,
			NULL);
		const char* textBytes = (const char*)textBuffer;
		if(count != 0)
			out = lux::core::UTF16ToString(textBytes);
		else
			out = "Unknown error";
		HeapFree(GetProcessHeap(), 0, textBuffer);
	}

	return out;
}

void conv_data(format::Context& ctx, const LogWin32Error& v, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);

	using namespace format;
	lux::string str = GetWin32ErrorString(v.error);
	format::CopyConvertAddString(ctx, StringType::Unicode, str.Data_c(), str.Size());
}

namespace lux
{

LRESULT WINAPI LuxDeviceWin32::WindowProc(HWND windowHandle,
	UINT uiMessage,
	WPARAM WParam,
	LPARAM LParam)
{
	LONG_PTR userData = GetWindowLongPtrW(windowHandle, GWLP_USERDATA);
	LuxDeviceWin32* luxDevice = reinterpret_cast<LuxDeviceWin32*>(userData);
	if(uiMessage == WM_NCCREATE) {
		luxDevice = reinterpret_cast<LuxDeviceWin32*>(reinterpret_cast<CREATESTRUCTW*>(LParam)->lpCreateParams);
		SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(luxDevice));

		StrongRef<gui::WindowWin32> windowWin32 = luxDevice->m_Window;
		windowWin32->Init(windowHandle);
	}

	if(luxDevice)
		return luxDevice->WinProc(windowHandle, uiMessage, WParam, LParam);
	else
		return DefWindowProcW(windowHandle, uiMessage, WParam, LParam);
}

LRESULT LuxDeviceWin32::WinProc(HWND windowHandle,
	UINT uiMessage,
	WPARAM WParam,
	LPARAM LParam)
{
	gui::WindowWin32* window = dynamic_cast<gui::WindowWin32*>(*m_Window);
	if(window && window->GetDeviceWindow() != windowHandle)
		window = nullptr;

	LRESULT result;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	if(m_RawInputReceiver && m_RawInputReceiver->HandleMessage(uiMessage, WParam, LParam, result))
		return result;
#endif

	if(window && window->HandleMessages(uiMessage, WParam, LParam, result))
		return result;

	switch(uiMessage) {
	case WM_QUIT:
		CloseDevice();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		BeginPaint(windowHandle, &Paint);

		EndPaint(windowHandle, &Paint);
		return 0;
	}

	case WM_ERASEBKGND:
		return 0;

	case WM_SYSCOMMAND:
		if((WParam & 0xFFF0) == SC_SCREENSAVE ||
			(WParam & 0xFFF0) == SC_MONITORPOWER)
			return 0;
		break;
	}

	return DefWindowProcW(windowHandle, uiMessage, WParam, LParam);
}

LUX_API StrongRef<LuxDevice> CreateDevice()
{
	StrongRef<LuxDevice> out = LUX_NEW(LuxDeviceWin32);
	return out;
}

LuxDeviceWin32::LuxDeviceWin32() :
	m_Time(0.0),
	m_Quit(false),
	m_InputEventProxy(this),
	m_LuxWindowClassName(WIN32_CLASS_NAME),
	m_WindowCallback(this)
{
	// If there are logs which aren't written, write them to the default file.
	if(log::EngineLog.HasUnsetLogs())
		log::EngineLog.SetNewPrinter(log::FilePrinter, true);

	m_Filesystem = LUX_NEW(io::FileSystemWin32);
	m_ReferableFactory = core::ReferableFactoryImpl::Instance();
	m_Window = LUX_NEW(gui::WindowWin32);
	m_ResourceSystem = LUX_NEW(core::ResourceSystemImpl)(m_Filesystem, m_ReferableFactory);

	log::Log("Starting time ~a", core::Clock::GetDateAndTime());

	m_ResourceSystem->AddType(core::ResourceType::Mesh);
	m_ResourceSystem->AddType(core::ResourceType::Image);
	m_ResourceSystem->AddType(core::ResourceType::ImageList);
	m_ResourceSystem->AddType(core::ResourceType::Texture);
	m_ResourceSystem->AddType(core::ResourceType::CubeTexture);
	m_ResourceSystem->AddType(core::ResourceType::Font);
	m_ResourceSystem->AddType(core::ResourceType::Sound);

	m_ResourceSystem->SetCaching(core::ResourceType::ImageList, false);
	m_ResourceSystem->SetCaching(core::ResourceType::Image, false);

	lux::core::impl::RunAllRegisterReferableFunctions();

	log::Info("Lux core was build.");
}

void LuxDeviceWin32::BuildWindow(u32 width, u32 height, const string& title)
{
	if(m_Window->GetDeviceWindow()) {
		log::Warning("Window already built.");
		return;
	}

	log::Info("Create new Lux window \"~s\".", title);

	CreateNewWindow(width, height, title);

	m_OwnWindow = true;

	m_Window->RegisterCallback(&m_WindowCallback);
}

void LuxDeviceWin32::SetOwnWindow(void* hOwnWindow)
{
	// Wenn bereits ein Fenster definiert ist no op
	if(m_Window->GetDeviceWindow()) {
		log::Warning("Window alread defined.");
		return;
	}

	HWND WinWindow = *((HWND*)hOwnWindow);
	if(WinWindow == nullptr || *((HWND*)WinWindow) == nullptr)
		throw core::InvalidArgumentException("hOwnWindow");

	StrongRef<gui::WindowWin32> windowWin32 = m_Window;
	windowWin32->Init(WinWindow);

	// Globalen Zeiger für Dlg-Funktion speichern
	// Globalen Zeiger für Dlg-Funktion speichern
	if(!GetWindowLongPtrW(WinWindow, GWLP_USERDATA)) {
		SetWindowLongPtrW(WinWindow, GWLP_USERDATA, (LONG_PTR)this);
		throw core::RuntimeException("Window userdata already used.");
	}
}

void LuxDeviceWin32::BuildInputSystem(bool isForeground)
{
	// If system already build -> no op
	if(m_InputSystem) {
		log::Warning("Input system alread built.");
		return;
	}

	if(!m_Window->GetDeviceWindow())
		throw core::Exception("Missing window");

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_InputSystem = LUX_NEW(input::InputSystemImpl)(isForeground);
	m_InputSystem->SetInputReceiver(&m_InputEventProxy);
	m_InputSystem->SetForegroundState(m_Window->IsFocused());

	m_RawInputReceiver = LUX_NEW(input::RawInputReceiver)(m_InputSystem, (HWND)m_Window->GetDeviceWindow());
	u32 keyboardCount = m_RawInputReceiver->DiscoverDevices(input::EEventSource::Keyboard);
	if(keyboardCount == 0)
		throw core::Exception("No keyboard found on system.");
#else
	throw core::NotImplementedException();
#endif

	log::Info("Built Input System.");
}

void LuxDeviceWin32::BuildVideoDriver(const video::DriverConfig& config)
{
	// If system already build -> no op
	if(m_Driver) {
		log::Warning("Videodriver already build.");
		return;
	}

	log::Info("Building Video Driver.");
#ifdef LUX_COMPILE_WITH_D3D9
	video::VideoDriverD3D9* driver = LUX_NEW(video::VideoDriverD3D9)(m_ReferableFactory);
	m_Driver = driver;
	m_Driver->Init(config, m_Window);

	BuildMaterials();

	driver->SetDefaultRenderer(m_MaterialLibrary->GetMaterialRenderer("solid"));
#else
	throw core::NotImplementedException();
#endif

	BuildImageSystem();
}

void LuxDeviceWin32::BuildMaterials()
{

#ifdef LUX_COMPILE_WITH_D3D9
	m_MaterialLibrary = LUX_NEW(video::MaterialLibraryImpl)(m_Driver, m_Filesystem);
	m_MaterialLibrary->AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_Solid_d3d9)(m_Driver), "solid");
	m_MaterialLibrary->AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_Solid_Mix_d3d9)(m_Driver), "solid_mix");
	m_MaterialLibrary->AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_OneTextureBlend_d3d9)(m_Driver), "transparent");
	m_MaterialLibrary->AddMaterialRenderer(LUX_NEW(video::CMaterialRenderer_VertexAlpha_d3d9)(m_Driver), "transparent_alpha");
#else
	throw core::NotImplementedException();
#endif

	video::MaterialRenderer* renderer = m_MaterialLibrary->GetMaterialRenderer("solid");
	video::IdentityMaterial.SetRenderer(renderer);
	video::WorkMaterial.SetRenderer(renderer);

	m_Driver->Set3DMaterial(video::IdentityMaterial);
	m_Driver->Set2DMaterial(video::IdentityMaterial);
}

void LuxDeviceWin32::BuildSceneManager()
{
	// If system already build -> no op
	if(m_SceneManager) {
		log::Warning("Scene Manager already built.");
		return;
	}

	if(!m_Driver)
		throw core::Exception("Missing video driver");

	if(!m_Filesystem)
		throw core::Exception("Missing file system");

	m_MeshSystem = LUX_NEW(scene::MeshSystemImpl)(m_ResourceSystem, m_Driver, m_MaterialLibrary);

	// Scene-Manager erstellen
	log::Info("Build Scene Manager.");
	m_SceneManager = LUX_NEW(scene::SceneManagerImpl)(
		m_Driver,
		m_ImageSystem,
		m_Filesystem,
		m_ReferableFactory,
		m_MeshSystem,
		m_ResourceSystem,
		m_MaterialLibrary);
}

void LuxDeviceWin32::BuildImageSystem()
{
	if(m_ImageSystem != nullptr) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Build Image System.");
	m_ImageSystem = LUX_NEW(video::ImageSystemImpl)(m_Filesystem, m_Driver, m_ResourceSystem);
}

void LuxDeviceWin32::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	if(!m_Driver) {
		throw core::Exception("Missing video driver");
		return;
	}

	log::Info("Build GUI Environment.");
	m_GUIEnv = LUX_NEW(gui::GUIEnvironmentImpl)(m_ResourceSystem, m_ImageSystem, m_Driver, m_MaterialLibrary, m_Filesystem);
}

void LuxDeviceWin32::BuildAll(const video::DriverConfig& config)
{
	BuildWindow(config.width, config.height, "Window");
	BuildInputSystem();
	BuildVideoDriver(config);
	BuildSceneManager();
	BuildGUIEnvironment();
}

// Erstellt ein neues Fenster
HWND LuxDeviceWin32::CreateNewWindow(u32 width, u32 height, const string& title)
{
	if(width > (u32)GetSystemMetrics(SM_CXSCREEN) || height > (u32)GetSystemMetrics(SM_CYSCREEN)) {
		throw core::Exception("The window is bigger than the screen and can't be created.");
	}

	WNDCLASSEXW wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0, 0,
		g_Instance, nullptr, LoadCursorW(NULL, IDC_ARROW), nullptr, nullptr,
		m_LuxWindowClassName, nullptr};

	if(!RegisterClassExW(&wc))
		throw core::Win32Exception(GetLastError());

	RECT rect;
	SetRect(&rect, 0, 0, width, height);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	HWND out = CreateWindowExW(0,
		m_LuxWindowClassName,
		core::StringToUTF16W(title),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2,
		width,
		height,
		nullptr,
		nullptr,
		wc.hInstance,
		this);

	if(!out)
		throw core::Win32Exception(GetLastError());

	return out;
}

LuxDeviceWin32::~LuxDeviceWin32()
{
	m_UserEventReceiver = nullptr;

	m_GUIEnv = nullptr;
	m_SceneManager = nullptr;

	m_ImageSystem = nullptr;
	m_MaterialLibrary = nullptr;

	m_Driver = nullptr;

	m_ResourceSystem = nullptr;

	m_Filesystem = nullptr;

	m_ReferableFactory = nullptr;

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver = nullptr;
#endif
	m_InputSystem = nullptr;

	m_Window = nullptr;

	// Fenster schließen
	if(m_OwnWindow)
		UnregisterClassW(m_LuxWindowClassName, g_Instance);

	log::Info("Shutdown complete.");

	log::EngineLog.Exit();
}

void LuxDeviceWin32::CloseDevice()
{
	m_Quit = true;
}

bool LuxDeviceWin32::HandleSystemMessages()
{
	MSG Message;
	memset(&Message, 0, sizeof(MSG));
	while(PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE)) {
		// No use for TranslateMessage since WM_CHAR is not used
		//TranslateMessage(&Message); 
		DispatchMessageW(&Message);
	}

	return m_Quit;
}

void LuxDeviceWin32::PostEvent(const input::Event& event, input::EEventTarget target)
{
	bool Handled = false;
	if(m_UserEventReceiver && !Handled && TestFlag(target, input::EEventTarget::User))
		Handled = m_UserEventReceiver->OnEvent(event);

	if(m_SceneManager && !Handled && TestFlag(target, input::EEventTarget::Scene))
		Handled = m_SceneManager->OnEvent(event);
}

void LuxDeviceWin32::SetUserEventReceiver(input::UserEventReceiver* receiver)
{
	m_UserEventReceiver = receiver;
}

StrongRef<input::UserEventReceiver> LuxDeviceWin32::GetUserEventReceiver() const
{
	return m_UserEventReceiver;
}

bool LuxDeviceWin32::Run(float& fNumSecsPassed)
{
	static auto StartTime = core::Clock::GetTicks();
	double Time;

	auto endTime = core::Clock::GetTicks();
	if(endTime == StartTime)
		Time = 0.000001;
	else
		Time = (double)(endTime - StartTime) * 0.001; // in Sekunden umrechnen

	fNumSecsPassed = (float)Time;
	m_Time += Time;

	StartTime = endTime;

	if(!m_Quit) {
		StrongRef<gui::WindowWin32> windowWin32 = m_Window;
		windowWin32->Tick();

		if(HandleSystemMessages()) {
			m_Window->Close();
			m_Window = nullptr;
		}
	}

	bool ret = m_Quit;
	m_Quit = false; // Restore quit flag.
	return !ret;
}


double LuxDeviceWin32::GetTime() const
{
	return m_Time;
}

StrongRef<video::VideoDriver> LuxDeviceWin32::GetDriver() const
{
	return m_Driver;
}

StrongRef<scene::SceneManager> LuxDeviceWin32::GetSceneManager() const
{
	return m_SceneManager;
}

StrongRef<input::InputSystem> LuxDeviceWin32::GetInputSystem() const
{
	return m_InputSystem;
}

StrongRef<io::FileSystem> LuxDeviceWin32::GetFileSystem() const
{
	return m_Filesystem;
}

StrongRef<video::ImageSystem> LuxDeviceWin32::GetImageSystem() const
{
	return m_ImageSystem;
}

StrongRef<gui::GUIEnvironment> LuxDeviceWin32::GetGUIEnvironment() const
{
	return m_GUIEnv;
}

StrongRef<video::MaterialLibrary> LuxDeviceWin32::GetMaterialLibrary() const
{
	return m_MaterialLibrary;
}

StrongRef<gui::Window> LuxDeviceWin32::GetWindow() const
{
	return m_Window;
}

StrongRef<core::ReferableFactory> LuxDeviceWin32::GetReferableFactory() const
{
	return m_ReferableFactory;
}

StrongRef<core::ResourceSystem> LuxDeviceWin32::GetResourceSystem() const
{
	return m_ResourceSystem;
}

StrongRef<scene::MeshSystem> LuxDeviceWin32::GetMeshSystem() const
{
	return m_MeshSystem;
}

}    //Namespace Lux  
#endif // LUX_WINDOWS
