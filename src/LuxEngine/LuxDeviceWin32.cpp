#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "LuxDeviceWin32.h"
#include "DllMainWin32.h"

#include "core/Clock.h"
#include "core/Logger.h"
#include "core/StringConverter.h"
#include "core/lxUnicodeConversion.h"

#include "input/InputSystem.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/raw_input/RawInputReceiver.h"
#endif

#include <WinUser.h>
#include "platform/Win32Exception.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/AdapterInformationD3D9.h"
#endif

namespace lux
{

static LRESULT WINAPI WindowProc(HWND wnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	LuxDeviceWin32* device = nullptr;
	if(msg == WM_NCCREATE) {
		CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
		SetWindowLongPtrW(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
		device = reinterpret_cast<LuxDeviceWin32*>(createStruct->lpCreateParams);
	} else {
		LONG_PTR userData = GetWindowLongPtrW(wnd, GWLP_USERDATA);
		device = reinterpret_cast<LuxDeviceWin32*>(userData);
	}

	LRESULT result;
	if(!device || !device->HandleMessages(wnd, msg, wParam, lParam, result))
		result = DefWindowProcW(wnd, msg, wParam, lParam);
	return result;
}

Win32WindowClass::Win32WindowClass()
{
	instance = lux::GetLuxModule();
	className = L"Lux Window Class";
	WNDCLASSEXW wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0, 0,
		instance, nullptr, NULL, nullptr, nullptr,
		className, nullptr};

	if(!RegisterClassExW(&wc))
		throw core::Win32Exception(GetLastError());
}
Win32WindowClass::~Win32WindowClass()
{
	UnregisterClassW(className, instance);
}

LUX_API StrongRef<LuxDevice> CreateDevice()
{
	return LUX_NEW(LuxDeviceWin32);
}

LuxDeviceWin32::LuxDeviceWin32()
{
	m_SysInfo = LUX_NEW(LuxSystemInfoWin32);
	m_NeverSetEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
#ifdef LUX_COMPILE_WITH_D3D9
	m_VideoDrivers[video::DriverType::Direct3D9] = VideoDriverEntry(
		[](const video::VideoDriverInitData& data) -> video::VideoDriver* { return LUX_NEW(video::VideoDriverD3D9)(data); },
		[]()                                       -> video::AdapterList* { return LUX_NEW(video::AdapterListD3D9); });
#endif
}

LuxDeviceWin32::~LuxDeviceWin32()
{
	CloseHandle(m_NeverSetEvent);

	ReleaseModules();

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver.Reset();
#endif
	input::InputSystem::Destroy();

	m_Window.Reset();

	log::Info("Shutdown complete.");
}

void LuxDeviceWin32::BuildWindow(int width, int height, core::StringView title)
{
	if(m_Window) {
		log::Warning("Window already built.");
		return;
	}

	log::Info("Create new Lux window \"~s\".", title);

	math::Dimension2I realSize(width, height);
	if(realSize.width > GetSystemMetrics(SM_CXSCREEN))
		realSize.width = GetSystemMetrics(SM_CXSCREEN);
	if(realSize.height > GetSystemMetrics(SM_CYSCREEN))
		realSize.height = GetSystemMetrics(SM_CYSCREEN);

	RECT rect;
	SetRect(&rect, 0, 0, realSize.width, realSize.height);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);
	realSize.width = rect.right - rect.left;
	realSize.height = rect.bottom - rect.top;

	HWND handle = CreateWindowExW(0,
		m_WindowClass.className,
		core::UTF8ToWin32String(title),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN) / 2 - realSize.width / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - realSize.height / 2,
		realSize.width,
		realSize.height,
		nullptr,
		nullptr,
		m_WindowClass.instance,
		this);
	if(!handle)
		throw core::Win32Exception(GetLastError());

	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);

	SetCursor(LoadCursorW(NULL, IDC_ARROW));
}

void LuxDeviceWin32::BuildInputSystem(bool isForeground)
{
	// If system already build -> no op
	if(input::InputSystem::Instance()) {
		log::Warning("Input system alread built.");
		return;
	}

	if(!m_Window->GetDeviceWindow())
		throw core::InvalidOperationException("Missing window");

	input::InputSystem::Initialize();
	auto inputSys = input::InputSystem::Instance();
	inputSys->SetDefaultForegroundHandling(isForeground);
	inputSys->SetForegroundState(m_Window->IsFocused());

#ifdef LUX_COMPILE_WITH_RAW_INPUT
	m_RawInputReceiver = LUX_NEW(input::RawInputReceiver)(inputSys, (HWND)m_Window->GetDeviceWindow());
	u32 keyboardCount = m_RawInputReceiver->DiscoverDevices(input::EEventSource::Keyboard);
	if(keyboardCount == 0)
		throw core::GenericRuntimeException("No keyboard found on system.");
#else
	throw core::NotImplementedException();
#endif

	log::Info("Built Input System.");
}

bool LuxDeviceWin32::RunMessageQueue(int waitTime)
{
	if(waitTime) {
		DWORD res = MsgWaitForMultipleObjects(1, &m_NeverSetEvent, FALSE, waitTime, QS_ALLINPUT);
		(void)res;
	}

	MSG Message = {0};
	while(PeekMessageW(&Message, (HWND)m_Window->GetDeviceWindow(), 0, 0, PM_REMOVE)) {
		if(Message.message == WM_QUIT)
			return true;
		// No use for TranslateMessage since WM_CHAR is not used
		//TranslateMessage(&Message); 
		DispatchMessageW(&Message);
	}

	return false;
}

bool LuxDeviceWin32::WaitForWindowChange()
{
	MSG Message = {0};
	while(true) {
		bool active = m_Window->IsActive();
		bool minimized = m_Window->IsMinimized();
		bool maximized = m_Window->IsMaximized();
		bool fullscreen = m_Window->IsFullscreen();
		bool focused = m_Window->IsFocused();

		BOOL result = GetMessageW(&Message, (HWND)m_Window->GetDeviceWindow(), 0, 0);
		if(result == -1)
			return false;
		if(Message.message == WM_QUIT) {
			// Put the quit message back into the queue
			PostQuitMessage(0);
			return false;
		}
		// No use for TranslateMessage since WM_CHAR is not used
		//TranslateMessage(&Message); 
		DispatchMessageW(&Message);
		if(active != m_Window->IsActive() ||
			minimized != m_Window->IsMinimized() ||
			maximized != m_Window->IsMaximized() ||
			fullscreen != m_Window->IsFullscreen() ||
			focused != m_Window->IsFocused())
			return true;
	}
}

bool LuxDeviceWin32::Run(int waitTime)
{
	bool wasQuit = false;
	if(m_Window) {
		if(RunMessageQueue(waitTime)) {
			m_Window->Close();
			wasQuit = true;
			m_Window = nullptr;
		}
	}

	return !wasQuit;
}

void LuxDeviceWin32::Sleep(int millis)
{
	::Sleep((DWORD)millis);
}

StrongRef<gui::Window> LuxDeviceWin32::GetWindow() const
{
	return m_Window;
}

StrongRef<LuxSystemInfo> LuxDeviceWin32::GetSystemInfo() const
{
	return m_SysInfo;
}

StrongRef<gui::Cursor> LuxDeviceWin32::GetCursor() const
{
	return m_Window->GetDeviceCursor();
}

bool LuxDeviceWin32::HandleMessages(
	HWND wnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam,
	LRESULT& result)
{
	if(message == WM_NCCREATE)
		m_Window = LUX_NEW(gui::WindowWin32)(wnd);

	result = 0;
#ifdef LUX_COMPILE_WITH_RAW_INPUT
	if(m_RawInputReceiver && m_RawInputReceiver->HandleMessage(message, wParam, lParam, result))
		return true;
#endif

	if(m_Window && m_Window->GetDeviceWindow() == wnd && m_Window->HandleMessages(message, wParam, lParam, result))
		return true;

	switch(message) {
	case WM_ERASEBKGND:
		return true;

	case WM_SYSCOMMAND:
		if((wParam & 0xFFF0) == SC_SCREENSAVE ||
			(lParam & 0xFFF0) == SC_MONITORPOWER)
			return true;
		break;
	}

	return false;
}

} // namespace lux

#endif // LUX_WINDOWS
