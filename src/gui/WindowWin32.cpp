#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "WindowWin32.h"

#include "gui/GUISkin.h"
#include "gui/GUIRenderer.h"

#include "video/images/Image.h"
#include "video/ColorConverter.h"
#include "core/Logger.h"

#include "core/lxUnicodeConversion.h"
#include "platform/Win32Exception.h"
#include "LuxEngine/DllMainWin32.h"

namespace lux
{
namespace gui
{

WindowWin32::WindowWin32(HWND window) :
	m_Window(window),
	m_IsFullscreen(false)
{
	SetClearBackground(false);

	m_Cursor = LUX_NEW(CursorWin32)(this);
	m_Beam = LoadCursorW(NULL, IDC_IBEAM);
	m_Wait = LoadCursorW(NULL, IDC_WAIT);
	m_Arrow = LoadCursorW(NULL, IDC_ARROW);

	BOOL isIconic = IsIconic(m_Window);
	BOOL isZoomed = IsZoomed(m_Window);
	if(isIconic)
		this->OnStateChange(EStateChange::Minimize);
	if(isZoomed)
		this->OnStateChange(EStateChange::Maximize);
	if(!isZoomed && !isIconic)
		this->OnStateChange(EStateChange::Normal);

	wchar_t text[200];
	int length;
	DWORD_PTR result;
	text[0] = 0;
	length = (int)SendMessageTimeoutW(m_Window, WM_GETTEXT,
		200, reinterpret_cast<LPARAM>(text),
		SMTO_ABORTIFHUNG, 2000, &result);
	core::String newTitle = core::UTF16ToString(text, -1);

	OnTitleChange(newTitle);
}

WindowWin32::~WindowWin32()
{
	if(m_Window)
		Close();
}

bool WindowWin32::SwitchFullscreen(bool fullscreen)
{
	if(fullscreen == m_IsFullscreen)
		return true;

	if(m_IsFullscreen == false) {
		m_SavedWindow.IsMaxed = IsMaximized();
		if(m_SavedWindow.IsMaxed)
			SendMessage(m_Window, WM_SYSCOMMAND, SC_RESTORE, 0);
		m_SavedWindow.Style = GetWindowLong(m_Window, GWL_STYLE);
		m_SavedWindow.ExStyle = GetWindowLong(m_Window, GWL_EXSTYLE);
		GetWindowRect(m_Window, &m_SavedWindow.WinRect);
	}

	m_IsFullscreen = fullscreen;
	if(m_IsFullscreen)
		this->OnStateChange(Window::EStateChange::Fullscreen);
	else
		if(m_SavedWindow.IsMaxed)
			this->OnStateChange(Window::EStateChange::Maximize);
		else
			this->OnStateChange(Window::EStateChange::Normal);

	if(fullscreen) {
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

void WindowWin32::Paint(Renderer* r)
{
	if(ClearBackground())
		r->DrawRectangle(GetFinalInnerRect(), GetFinalPalette().GetWindow());
}

void WindowWin32::SetText(const core::String& text)
{
	auto data = core::UTF8ToWin32String(text);
	DWORD_PTR result;
	SendMessageTimeoutW(m_Window, WM_SETTEXT, 0,
		reinterpret_cast<LPARAM>(data.Data()),
		SMTO_ABORTIFHUNG, 500, &result);
	LUX_UNUSED(result);
}

void WindowWin32::SetInnerSize(const ScalarDimensionF& size)
{
	auto screen = GetParentInnerRect();
	math::Dimension2I real(
		(u32)size.width.GetRealValue(screen.GetWidth()),
		(u32)size.height.GetRealValue(screen.GetHeight()));

	RECT rect;
	SetRect(&rect, 0, 0, real.width, real.height);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	real.width = rect.right - rect.left;
	real.height = rect.bottom - rect.top;

	SetSize(Pixel(real.width), Pixel(real.height));
}

math::RectF WindowWin32::GetParentInnerRect() const
{
	POINT p;
	p.x = p.y = 0;
	ClientToScreen(m_Window, &p);
	return math::RectF(
		-(float)p.x, -(float)p.y,
		(float)(GetSystemMetrics(SM_CXSCREEN) - p.x),
		(float)(GetSystemMetrics(SM_CYSCREEN) - p.y));
}

bool WindowWin32::UpdateFinalRect()
{
	POINT p;
	p.x = p.y = 0;
	ClientToScreen(m_Window, &p);

	auto oldRect = m_WindowScreenRect;
	WindowBase::UpdateFinalRect();
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_Window, &plc);
	plc.rcNormalPosition.left = (LONG)m_FinalRect.left + p.x;
	plc.rcNormalPosition.top = (LONG)m_FinalRect.top + p.y;
	plc.rcNormalPosition.right = plc.rcNormalPosition.left + (LONG)m_FinalRect.GetWidth();
	plc.rcNormalPosition.bottom = plc.rcNormalPosition.top + (LONG)m_FinalRect.GetHeight();
	SetWindowPlacement(m_Window, &plc);
	return std::memcmp(&m_WindowScreenRect, &oldRect, sizeof(RECT)) != 0;
}

bool WindowWin32::UpdateInnerRect()
{
	return true;
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

void WindowWin32::Close()
{
	DestroyWindow(m_Window);
}

bool WindowWin32::Present(
	video::Image* image,
	const math::RectI& _sourceRect,
	const math::RectI& _destRect)
{
	if(!image)
		return true;

	HDC dc = GetDC(m_Window);
	RECT rect;
	GetClientRect(m_Window, &rect);
	video::ImageLock lock(image);
	const void* mem = lock.data;
	if(!mem)
		return false;
	video::ColorFormat format = image->GetColorFormat();

	math::RectI dstRect = _destRect;
	math::RectI winRect = math::RectI(
		rect.left, rect.top,
		rect.right, rect.bottom);
	if(dstRect.IsEmpty())
		dstRect = winRect;
	else
		dstRect.FitInto(winRect);

	math::RectI srcRect = _sourceRect;
	math::RectI imageRect = math::RectI(
		0, 0,
		image->GetSize().width, image->GetSize().height);
	if(srcRect.IsEmpty())
		srcRect = imageRect;
	else
		srcRect.FitInto(imageRect);

	math::Dimension2<int> imageDim = imageRect.GetSize();
	core::RawMemory data;
	auto bitsPerPixel = image->GetColorFormat().GetBitsPerPixel();
	if(bitsPerPixel == 8) {
		// Immer nach ARGB
		data.SetSize(4 * (bitsPerPixel*srcRect.GetArea())/8);
		video::ColorConverter::ConvertByFormat(
			mem, image->GetColorFormat(),
			data, video::ColorFormat::A8R8G8B8,
			srcRect.GetWidth(), srcRect.GetHeight(),
			lock.pitch, srcRect.GetWidth() * 4);
		mem = data;
		format = video::ColorFormat::A8R8G8B8;
		imageDim.width = srcRect.GetWidth();
		imageDim.height = srcRect.GetHeight();
	} else if(bitsPerPixel == 24) {
		// Immer nach ARGB
		data.SetSize((bitsPerPixel*srcRect.GetArea())/8);
		video::ColorConverter::ConvertByFormat(
			mem, image->GetColorFormat(),
			data, video::ColorFormat::A8R8G8B8,
			srcRect.GetWidth(), srcRect.GetHeight(),
			lock.pitch, srcRect.GetWidth() * 4);
		mem = data;
		format = video::ColorFormat::A8R8G8B8;
		imageDim.width = srcRect.GetWidth();
		imageDim.height = srcRect.GetHeight();
	} else if(bitsPerPixel == 16 || bitsPerPixel == 32) {
		/*
		NO OP
		*/
	} else {
		log::Error("Can't display image in this format.({})", format);
		return false;
	}

	BITMAPV4HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV4Size = sizeof(BITMAPV4HEADER);
	bi.bV4BitCount = (WORD)format.GetBitsPerPixel();
	bi.bV4Planes = 1;
	bi.bV4Width = imageDim.width;
	bi.bV4Height = -((int)imageDim.height);
	bi.bV4V4Compression = BI_BITFIELDS;
	bi.bV4AlphaMask = format.GetAlphaMask();
	bi.bV4RedMask = format.GetRedMask();
	bi.bV4GreenMask = format.GetGreenMask();
	bi.bV4BlueMask = format.GetBlueMask();

	StretchDIBits(dc,
		dstRect.left, dstRect.top, dstRect.GetWidth(), dstRect.GetHeight(),
		srcRect.left, srcRect.top, srcRect.GetWidth(), srcRect.GetHeight(),
		mem,
		(const BITMAPINFO*)&bi, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(m_Window, dc);

	return true;
}

void WindowWin32::Restore()
{
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	if(!GetWindowPlacement(m_Window, &plc))
		return;

	plc.showCmd = SW_SHOWNORMAL;
	if(!SetWindowPlacement(m_Window, &plc))
		return;
}

void* WindowWin32::GetDeviceWindow() const
{
	return m_Window;
}

Cursor* WindowWin32::GetDeviceCursor() const
{
	return m_Cursor;
}

core::Name WindowWin32::GetReferableType() const
{
	static const core::Name name("lux.gui.SystemWindow");
	return name;
}

bool WindowWin32::HandleMessages(UINT Message,
	WPARAM WParam,
	LPARAM LParam,
	LRESULT& result)
{
	result = 0;

	switch(Message) {
	case WM_MOUSEMOVE:
		if(m_Cursor->IsGrabbing()) {
			POINTS p = MAKEPOINTS(LParam);
			math::Vector2F pos = m_Cursor->m_GrabbingPosition;
			if(p.x != pos.x || p.y != pos.y)
				m_Cursor->SetPosition(pos.x, pos.y);

			result = 0;
			return true;
		} else {
			m_Cursor->Tick();
		}
		break;
	case WM_SETCURSOR:
		if(LOWORD(LParam) == HTCLIENT && !m_Cursor->IsVisible()) {
			SetCursor(NULL);
			result = TRUE;
			return true;
		} else {
			HCURSOR cursor;
			switch(m_Cursor->GetState()) {
			case ECursorState::Beam: cursor = m_Beam; break;
			case ECursorState::Wait: cursor = m_Wait; break;
			case ECursorState::Normal: cursor = m_Arrow; break;
			default: cursor = m_Arrow; break;
			}
			SetCursor(cursor);
			result = TRUE;
			return true;
		}
		break;
	case WM_DESTROY:
		onClose.Broadcast(this);
		m_Window = NULL;
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		Close();
		break;
	case WM_MOVE:
		// Perform the resize
		result = DefWindowProcW(m_Window, Message, WParam, LParam);

		RectChange();
		return true;
		break;
	case WM_SETTEXT:
		OnTitleChange(core::UTF16ToString((void*)LParam, -1));
		break;
	case WM_SIZE:
	{
		// Perform the resize
		result = DefWindowProcW(m_Window, Message, WParam, LParam);

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
		RectChange();

		return true;
	}
	break;
	case WM_SETFOCUS:
		if(m_Cursor->IsGrabbing()) {
			// Regrab the cursor.
			auto grabPos = m_Cursor->m_GrabbingPosition;
			m_Cursor->UnGrabCursor();
			m_Cursor->GrabCursor(grabPos);
		}
		OnStateChange(EStateChange::FocusGained);
		break;

	case WM_KILLFOCUS:
		OnStateChange(EStateChange::FocusLost);
		break;

	case WM_ENABLE:
		OnStateChange(EStateChange::Activated);
		break;

	case WM_CANCELMODE:
		OnStateChange(EStateChange::Deactivated);
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

#endif // LUX_WINDOWS
