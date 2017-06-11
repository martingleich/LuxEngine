#ifdef LUX_WINDOWS
#include "LuxDeviceWin32.h"

#include "core/Clock.h"
#include "core/Logger.h"
#include "core/lxRandom.h"
#include "core/StringConverter.h"
#include "core/ReferableFactory.h"

#include "resources/ResourceSystem.h"

#include "scene/SceneManagerImpl.h"

#include "io/FileSystem.h"

#include "gui/WindowWin32.h"
#include "gui/CursorControlWin32.h"
#include "gui/GUIEnvironmentImpl.h"

#include "core/lxUnicodeConversion.h"

#include "core/ReferableRegister.h"

#include "video/mesh/MeshSystem.h"
#include "video/MaterialLibrary.h"

#include "video/images/ImageSystemImpl.h"

#include "video/VideoDriver.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#endif
#include "video/Renderer.h"

#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/raw_input/RawInputReceiver.h"
#endif

#include <WinUser.h>
#include "LuxEngine/Win32Exception.h"

namespace lux
{
// Validate sizes and offsets of data layout types.
// TODO: Move this to a more platform indepented place
static_assert(offsetof(math::vector2f, x) == 0, "Bad offset");
static_assert(offsetof(math::vector2f, y) == 4, "Bad offset");
static_assert(sizeof(math::vector2f) == 8, "Bad size");

static_assert(offsetof(math::vector3f, x) == 0, "Bad offset");
static_assert(offsetof(math::vector3f, y) == 4, "Bad offset");
static_assert(offsetof(math::vector3f, z) == 8, "Bad offset");
static_assert(sizeof(math::vector3f) == 12, "Bad size");

static_assert(offsetof(math::quaternionf, x) == 0, "Bad offset");
static_assert(offsetof(math::quaternionf, y) == 4, "Bad offset");
static_assert(offsetof(math::quaternionf, z) == 8, "Bad offset");
static_assert(offsetof(math::quaternionf, w) == 12, "Bad offset");
static_assert(sizeof(math::quaternionf) == 16, "Bad size");

static_assert(offsetof(video::Colorf, r) == 0, "Bad offset");
static_assert(offsetof(video::Colorf, g) == 4, "Bad offset");
static_assert(offsetof(video::Colorf, b) == 8, "Bad offset");
static_assert(offsetof(video::Colorf, a) == 12, "Bad offset");
static_assert(sizeof(video::Colorf) == 16, "Bad size");

static_assert(sizeof(math::matrix4) == 4 * 4 * 4, "Bad size");
static_assert(sizeof(video::Color) == 4, "Bad size");
static_assert(sizeof(math::vector2i) == 8, "Bad size");
static_assert(sizeof(math::vector3i) == 12, "Bad size");
}

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
	m_LuxWindowClassName(WIN32_CLASS_NAME),
	m_WindowCallback(this)
{
	// If there are logs which aren't written, write them to the default file.
	if(log::EngineLog.HasUnsetLogs())
		log::EngineLog.SetNewPrinter(log::FilePrinter, true);

	// Create the singleton classes
	io::FileSystem::Initialize();
	core::ReferableFactory::Initialize();
	core::ResourceSystem::Initialize();

	auto resourceSystem = core::ResourceSystem::Instance();

	m_Window = LUX_NEW(gui::WindowWin32);

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

	// Globalen Zeiger f¸r Dlg-Funktion speichern
	// Globalen Zeiger f¸r Dlg-Funktion speichern
	if(!GetWindowLongPtrW(WinWindow, GWLP_USERDATA)) {
		SetWindowLongPtrW(WinWindow, GWLP_USERDATA, (LONG_PTR)this);
		throw core::RuntimeException("Window userdata already used.");
	}
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

	video::VideoDriver* driver;

#ifdef LUX_COMPILE_WITH_D3D9
	video::VideoDriverD3D9* d3d9Driver = LUX_NEW(video::VideoDriverD3D9);
	d3d9Driver->Init(config, m_Window);
	driver = d3d9Driver;
	video::VideoDriver::Initialize(driver);
#endif

	if(!driver)
		throw core::NotImplementedException();

	video::MaterialLibrary::Initialize();

	auto invalidMaterial = video::MaterialLibrary::Instance()->CreateMaterial("debug_overlay");
	invalidMaterial->SetDiffuse(video::Color(255, 0, 255));
	driver->GetRenderer()->SetInvalidMaterial(invalidMaterial);

	BuildImageSystem();

	video::MeshSystem::Initialize();
}

void LuxDeviceWin32::BuildSceneManager()
{
	// If system already build -> no op
	if(m_SceneManager) {
		log::Warning("Scene Manager already built.");
		return;
	}

	log::Info("Build Scene Manager.");
	m_SceneManager = LUX_NEW(scene::SceneManagerImpl)(m_ImageSystem);
}

void LuxDeviceWin32::BuildImageSystem()
{
	if(m_ImageSystem != nullptr) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Build Image System.");
	m_ImageSystem = LUX_NEW(video::ImageSystemImpl);
}

void LuxDeviceWin32::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Build GUI Environment.");
	m_GUIEnv = LUX_NEW(gui::GUIEnvironmentImpl)(m_ImageSystem);
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
	m_GUIEnv = nullptr;
	m_SceneManager = nullptr;

	m_ImageSystem = nullptr;

	video::VideoDriverD3D9* driver = dynamic_cast<video::VideoDriverD3D9*>(video::VideoDriver::Instance());

	// Free all shared resources, default materials, invalid materials and so on
	if(driver)
		driver->CleanUp();

	video::MaterialLibrary::Destroy();
	video::VideoDriver::Destroy();

	core::ResourceSystem::Destroy();
	core::ReferableFactory::Destroy();
	io::FileSystem::Destroy();

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver.Reset();
#endif

	input::InputSystem::Destroy();

	m_Window = nullptr;

	// Fenster schlieﬂen
	if(m_OwnWindow)
		UnregisterClassW(m_LuxWindowClassName, g_Instance);

	log::Info("Shutdown complete.");
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

StrongRef<scene::SceneManager> LuxDeviceWin32::GetSceneManager() const
{
	return m_SceneManager;
}

StrongRef<video::ImageSystem> LuxDeviceWin32::GetImageSystem() const
{
	return m_ImageSystem;
}

StrongRef<gui::GUIEnvironment> LuxDeviceWin32::GetGUIEnvironment() const
{
	return m_GUIEnv;
}

StrongRef<gui::Window> LuxDeviceWin32::GetWindow() const
{
	return m_Window;
}

} // namespace lux

#endif // LUX_WINDOWS
