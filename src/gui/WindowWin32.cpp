#include "WindowWin32.h"
#include "video/images/Image.h"
#include "video/ColorConverter.h"
#include "core/Logger.h"

#include "CursorControlWin32.h"
#include "core/lxUnicodeConversion.h"

namespace lux
{
namespace gui
{

bool WindowWin32::SwitchFullscreen(bool Fullscreen)
{
#if 0
	/*
	if(m_ShouldFullscreen == false)
		return true;
*/
	math::dimension2du FullscreenSize = GetSize();
	u32 FullscreenBits = 32;

	if(Fullscreen == false) {
		if(m_IsFullscreen) {
			return (ChangeDisplaySettings(&m_DesktopMode, 0) == DISP_CHANGE_SUCCESSFUL);
		} else {
			return true;
		}
	}

	DEVMODE dm;
	memset(&dm, 0, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	dm.dmPelsWidth = FullscreenSize.width;
	dm.dmPelsHeight = FullscreenSize.Height;
	dm.dmBitsPerPel = FullscreenBits;
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

	LONG result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	if(result != DISP_CHANGE_SUCCESSFUL) {
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	}

	bool ret = false;
	switch(result) {
	case DISP_CHANGE_SUCCESSFUL:
		m_IsFullscreen = true;
		ret = true;
		break;
	case DISP_CHANGE_RESTART:
		log::Error("Switch to fullscreen: The computer must be restarted in order for the graphics mode to work.");
		break;
	case DISP_CHANGE_BADFLAGS:
		log::Error("Switch to fullscreen: An invalid set of flags was passed in.");
		break;
	case DISP_CHANGE_BADPARAM:
		log::Error("Switch to fullscreen: An invalid parameter was passed in. This can include an invalid flag or combination of flags.");
		break;
	case DISP_CHANGE_FAILED:
		log::Error("Switch to fullscreen: The display driver failed the specified graphics mode.");
		break;
	case DISP_CHANGE_BADMODE:
		log::Error("Switch to fullscreen: The graphics mode is not supported.");
		break;
	default:
		log::Error("An unknown error occured while changing to fullscreen.");
		break;
	}

	return ret;
#endif

	if(Fullscreen == m_IsFullscreen)
		return true;

	if(m_IsFullscreen == false) {
		m_SavedWindow.IsMaxed = IsMaximized();
		if(m_SavedWindow.IsMaxed)
			SendMessage(m_Window, WM_SYSCOMMAND, SC_RESTORE, 0);
		m_SavedWindow.Style = GetWindowLong(m_Window, GWL_STYLE);
		m_SavedWindow.ExStyle = GetWindowLong(m_Window, GWL_EXSTYLE);
		GetWindowRect(m_Window, &m_SavedWindow.WinRect);
	}

	m_IsFullscreen = Fullscreen;
	if(m_IsFullscreen)
		this->OnStateChange(Window::EStateChange::Fullscreen);
	else
		if(m_SavedWindow.IsMaxed)
			this->OnStateChange(Window::EStateChange::Maximize);
		else
			this->OnStateChange(Window::EStateChange::Normal);

	if(Fullscreen) {
		SetWindowLong(m_Window, GWL_STYLE, m_SavedWindow.Style & ~(WS_CAPTION | WS_THICKFRAME));
		SetWindowLong(m_Window, GWL_EXSTYLE, m_SavedWindow.ExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

		MONITORINFO MonitorInfo;
		MonitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST), &MonitorInfo);

		SetWindowPos(m_Window, NULL,
			MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
			MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
			MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	} else {
		SetWindowLong(m_Window, GWL_STYLE, m_SavedWindow.Style);
		SetWindowLong(m_Window, GWL_EXSTYLE, m_SavedWindow.ExStyle);

		SetWindowPos(m_Window, NULL, m_SavedWindow.WinRect.left, m_SavedWindow.WinRect.top,
			m_SavedWindow.WinRect.right - m_SavedWindow.WinRect.left,
			m_SavedWindow.WinRect.bottom - m_SavedWindow.WinRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		if(m_SavedWindow.IsMaxed)
			SendMessage(m_Window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}

	return true;
}

WindowWin32::WindowWin32() : m_Window(0)
{
	m_CursorControl = LUX_NEW(CursorControlWin32)(this);
}

WindowWin32::~WindowWin32()
{
	if(m_Window)
		Close();
}

bool WindowWin32::Init(HWND Window)
{
	m_Window = Window;
	m_IsFullscreen = false;

#if 0
	memset(&m_DesktopMode, 0, sizeof(DEVMODE));
	m_DesktopMode.dmSize = sizeof(DEVMODE);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &m_DesktopMode);
#endif

	RECT r;
	if(GetWindowRect(m_Window, &r)) {
		this->OnResize(r.right - r.left, r.bottom - r.top);
		this->OnMove(r.left, r.top);
	}

	BOOL isIconic = IsIconic(m_Window);
	BOOL isZoomed = IsZoomed(m_Window);
	if(isIconic)
		this->OnStateChange(EStateChange::Minimize);
	if(isZoomed)
		this->OnStateChange(EStateChange::Maximize);
	if(!isZoomed && !isIconic)
		this->OnStateChange(EStateChange::Normal);

	wchar_t text[200];
	size_t length;
	DWORD_PTR result;
	text[0] = 0;
	length = (size_t)SendMessageTimeoutW(m_Window, WM_GETTEXT,
		200, reinterpret_cast<LPARAM>(text),
		SMTO_ABORTIFHUNG, 2000, &result);
	string newTitle = core::UTF16ToString(text);

	OnTitleChange(newTitle);

	return true;
}

void WindowWin32::SetTitle(const string& title)
{
	auto data = core::UTF8ToUTF16(title.Data());
	DWORD_PTR result;
	SendMessageTimeoutW(m_Window, WM_SETTEXT, 0,
		reinterpret_cast<LPARAM>(data.Data_c()),
		SMTO_ABORTIFHUNG, 2000, &result);
}

void WindowWin32::SetSize(const math::dimension2du& Size)
{

	RECT rect;
	SetRect(&rect, 0, 0, Size.width, Size.height);
	// TODO: Get Correct flags from window
	// TODO: Einheitliches verhalten bei fenstergröße
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	u32 width = rect.right - rect.left;
	u32 height = rect.bottom - rect.top;

	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_Window, &plc);
	plc.rcNormalPosition.right = plc.rcNormalPosition.left + width;
	plc.rcNormalPosition.bottom = plc.rcNormalPosition.top + height;
	SetWindowPlacement(m_Window, &plc);
}

void WindowWin32::SetPosition(const math::vector2i& Position)
{
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_Window, &plc);
	int width = plc.rcNormalPosition.right - plc.rcNormalPosition.left;
	int height = plc.rcNormalPosition.bottom - plc.rcNormalPosition.top;
	plc.rcNormalPosition.left = Position.x;
	plc.rcNormalPosition.right = Position.x + width;
	plc.rcNormalPosition.top = Position.y;
	plc.rcNormalPosition.bottom = Position.y + height;
	SetWindowPlacement(m_Window, &plc);
}

bool WindowWin32::Maximize()
{
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	if(!GetWindowPlacement(m_Window, &plc))
		return false;
	plc.showCmd = SW_SHOWMAXIMIZED;
	if(!SetWindowPlacement(m_Window, &plc))
		return false;
	return true;
}

bool WindowWin32::Minimize()
{
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	if(!GetWindowPlacement(m_Window, &plc))
		return false;
	plc.showCmd = SW_SHOWMINNOACTIVE;
	if(!SetWindowPlacement(m_Window, &plc))
		return false;
	return true;
}

bool WindowWin32::SetResizable(bool resize)
{
	const LONG style = GetWindowLongW(m_Window, GWL_STYLE);
	LONG newStyle;
	if(resize)
		newStyle = style | WS_SIZEBOX | WS_MAXIMIZEBOX;
	else
		newStyle = style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX);

	SetLastError(0);
	if(SetWindowLongW(m_Window, GWL_STYLE, newStyle) == 0) {
		if(GetLastError() != 0)
			return false;
	}

	return true;
}

bool WindowWin32::Close()
{
	DWORD_PTR result;
	SendMessageTimeoutA(m_Window, WM_CLOSE,
		0, 0,
		SMTO_ABORTIFHUNG, 2000,
		&result);
	m_Window = NULL;
	return true;
}

bool WindowWin32::Present(video::Image* image, const math::recti& SourceRect, const math::recti& DestRect)
{
	if(!image)
		return true;

	HDC DC = GetDC(m_Window);
	RECT rect;
	GetClientRect(m_Window, &rect);
	const void* mem = (const void*)image->Lock();
	if(!mem)
		return false;
	video::ColorFormat Format = image->GetColorFormat();

	math::recti DstRect = DestRect;
	math::recti WinRect = math::recti(rect.left, rect.top, rect.right, rect.bottom);
	if(DestRect.IsEmpty())
		DstRect = WinRect;
	else
		DstRect.FitInto(WinRect);

	math::recti SrcRect = SourceRect;
	math::recti ImageRect = math::recti(0, 0, image->GetDimension().width, image->GetDimension().height);
	if(SrcRect.IsEmpty())
		SrcRect = ImageRect;
	else
		SrcRect.FitInto(ImageRect);

	math::dimension2d<int> ImageDim = ImageRect.GetDimension();
	void* data = nullptr;
	if(image->GetBitsPerPixel() == 8) {
		// Immer nach ARGB
		data = LUX_NEW_ARRAY(u8, 4 * image->GetBytesPerPixel()*SrcRect.GetArea());
		video::ColorConverter::ConvertByFormat(mem, image->GetColorFormat(), data, video::ColorFormat::A8R8G8B8, SrcRect.GetWidth(), SrcRect.GetHeight(),
			image->GetPitch(), SrcRect.GetWidth() * 4);
		mem = data;
		Format = video::ColorFormat::A8R8G8B8;
		ImageDim.width = SrcRect.GetWidth();
		ImageDim.height = SrcRect.GetHeight();
	} else if(image->GetBitsPerPixel() == 24) {
		// Immer nach ARGB
		data = LUX_NEW_ARRAY(u8, 4 * image->GetBytesPerPixel()*SrcRect.GetArea());
		video::ColorConverter::ConvertByFormat(mem, image->GetColorFormat(), data, video::ColorFormat::A8R8G8B8, SrcRect.GetWidth(), SrcRect.GetHeight(),
			image->GetPitch(), SrcRect.GetWidth() * 4);
		mem = data;
		Format = video::ColorFormat::A8R8G8B8;
		ImageDim.width = SrcRect.GetWidth();
		ImageDim.height = SrcRect.GetHeight();
	} else if(image->GetBitsPerPixel() == 16 || image->GetBitsPerPixel() == 32) {
		/*
		NO OP
		*/
	} else {
		log::Error("Can't display image in this format.(~a)", Format);
		return false;
	}

	BITMAPV4HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV4Size = sizeof(BITMAPV4HEADER);
	bi.bV4BitCount = (WORD)Format.GetBitsPerPixel();
	bi.bV4Planes = 1;
	bi.bV4Width = ImageDim.width;
	bi.bV4Height = -((int)ImageDim.height);
	bi.bV4V4Compression = BI_BITFIELDS;
	bi.bV4AlphaMask = Format.GetAlphaMask();
	bi.bV4RedMask = Format.GetRedMask();
	bi.bV4GreenMask = Format.GetGreenMask();
	bi.bV4BlueMask = Format.GetBlueMask();

	StretchDIBits(DC,
		DstRect.Left, DstRect.Top, DstRect.GetWidth(), DstRect.GetHeight(),
		SrcRect.Left, SrcRect.Top, SrcRect.GetWidth(), SrcRect.GetHeight(),
		mem,
		(const BITMAPINFO*)&bi, DIB_RGB_COLORS, SRCCOPY);

	LUX_FREE_ARRAY(data);

	image->Unlock();
	ReleaseDC(m_Window, DC);

	return true;
}

bool WindowWin32::Restore()
{
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	if(!GetWindowPlacement(m_Window, &plc))
		return false;

	plc.showCmd = SW_SHOWNORMAL;
	if(!SetWindowPlacement(m_Window, &plc))
		return false;

	return true;
}

void* WindowWin32::GetDeviceWindow() const
{
	return m_Window;
}

WeakRef<CursorControl> WindowWin32::GetCursorControl()
{
	return m_CursorControl.GetWeak();
}

void WindowWin32::Tick()
{
	//static_cast<CursorControlWin32*>(*m_CursorControl)->Tick();
}

bool WindowWin32::HandleMessages(UINT Message,
	WPARAM WParam,
	LPARAM LParam,
	LRESULT& result)
{
	result = 0;

	switch(Message) {
	case WM_MOUSEMOVE:
		if(m_CursorControl->IsGrabbing()) {
			POINTS p = MAKEPOINTS(LParam);
			math::vector2i pos = m_CursorControl->GetGrabbingPosition();
			if(p.x != pos.x || p.y != pos.y)
				m_CursorControl->SetPosition(pos.x, pos.y);

			result = 0;
			return true;
		}
		break;
	case WM_DESTROY:
		OnClose();
		break;
	case WM_CLOSE:
		if(OnClosing())
			DestroyWindow(m_Window);
		break;
	case WM_MOVE:
		OnMove(LOWORD(LParam), HIWORD(LParam));
		break;
	case WM_SETTEXT:
		OnTitleChange(core::UTF16ToString((void*)LParam));
		break;
	case WM_SIZE:
	{
		EStateChange state;
		switch(WParam) {
		case SIZE_MAXIMIZED:
			state = EStateChange::Maximize;
			break;
		case SIZE_MINIMIZED:
			state = EStateChange::Minimize;
			break;
		case SIZE_RESTORED:
			state = EStateChange::Normal;
			break;
		default:
			state = EStateChange::Normal;
			break;
		}
		OnStateChange(state);
		OnResize(LOWORD(LParam), HIWORD(LParam));
	}
	break;
	case WM_SETFOCUS:
		OnStateChange(EStateChange::FocusGained);
		break;

	case WM_KILLFOCUS:
		OnStateChange(EStateChange::FocusLost);
		break;

	case WM_ACTIVATE:
		if(WParam == WA_ACTIVE || WParam == WA_CLICKACTIVE)
			OnStateChange(EStateChange::Activated);
		else if(WParam == WA_INACTIVE)
			OnStateChange(EStateChange::Deactivated);
		break;
	}

	return false;
}

} 

} 

