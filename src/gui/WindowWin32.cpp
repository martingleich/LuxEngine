#ifdef LUX_WINDOWS
#include "WindowWin32.h"

#include "gui/GUISkin.h"
#include "gui/GUIRenderer.h"

#include "video/images/Image.h"
#include "video/ColorConverter.h"
#include "core/Logger.h"

#include "core/lxUnicodeConversion.h"
#include "Win32Exception.h"

namespace lux
{
namespace gui
{

WindowWin32::WindowWin32(HWND window)
{
	m_Window = window;
	m_IsFullscreen = false;
	SetClearBackground(false);

	m_Cursor = LUX_NEW(CursorWin32)(this);

	RECT r;
	if(GetWindowRect(m_Window, &r)) {
		OnResize((float)(r.right - r.left), (float)(r.bottom - r.top));
		OnMove((float)r.left, (float)r.top);
	}

	BOOL isIconic = IsIconic(m_Window);
	BOOL isZoomed = IsZoomed(m_Window);
	if(isIconic)
		this->OnStateChange(EStateChange::Minimize);
	if(isZoomed)
		this->OnStateChange(EStateChange::Maximize);
	if(!isZoomed && !isIconic)
		this->OnStateChange(EStateChange::Normal);

	u16 text[200];
	size_t length;
	DWORD_PTR result;
	text[0] = 0;
	length = (size_t)SendMessageTimeoutW(m_Window, WM_GETTEXT,
		200, reinterpret_cast<LPARAM>(text),
		SMTO_ABORTIFHUNG, 2000, &result);
	String newTitle = core::UTF16ToString(text);

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

void WindowWin32::SetText(const String& text)
{
	auto data = core::UTF8ToUTF16(text.Data());
	DWORD_PTR result;
	LRESULT r = SendMessageTimeoutW(m_Window, WM_SETTEXT, 0,
		reinterpret_cast<LPARAM>(data.Data_c()),
		SMTO_ABORTIFHUNG, 500, &result);

	if(r == 0)
		WindowBase::SetText(text);
}

void WindowWin32::SetInnerSize(const ScalarDimensionF& size)
{
	u32 realWidth = (u32)size.width.GetRealValue((float)GetSystemMetrics(SM_CXSCREEN));
	u32 realHeight = (u32)size.height.GetRealValue((float)GetSystemMetrics(SM_CYSCREEN));

	RECT rect;
	SetRect(&rect, 0, 0, realWidth, realHeight);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	realWidth = rect.right - rect.left;
	realHeight = rect.bottom - rect.top;

	SetSize(Pixel(realWidth), Pixel(realHeight));
}

math::RectF WindowWin32::GetParentInnerRect() const
{
	return math::RectF(0, 0,
		(float)GetSystemMetrics(SM_CXSCREEN),
		(float)GetSystemMetrics(SM_CYSCREEN));
}

bool WindowWin32::UpdateFinalRect()
{
	auto oldSize = m_FinalRect.GetSize();
	auto oldPos = m_FinalRect.LeftTop();
	WindowBase::UpdateFinalRect();
	WINDOWPLACEMENT plc;
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_Window, &plc);
	plc.rcNormalPosition.left = (LONG)m_FinalRect.left;
	plc.rcNormalPosition.top = (LONG)m_FinalRect.top;
	plc.rcNormalPosition.right = (LONG)m_FinalRect.right;
	plc.rcNormalPosition.bottom = (LONG)m_FinalRect.bottom;
	SetWindowPlacement(m_Window, &plc);
	GetWindowPlacement(m_Window, &plc);
	m_FinalRect.left = (float)plc.rcNormalPosition.left;
	m_FinalRect.top = (float)plc.rcNormalPosition.top;
	m_FinalRect.right = (float)plc.rcNormalPosition.right;
	m_FinalRect.bottom = (float)plc.rcNormalPosition.bottom;

	auto newSize = m_FinalRect.GetSize();
	auto newPos = m_FinalRect.LeftTop();
	bool change = false;
	if(oldSize != newSize) {
		change = true;
		onResize.Broadcast(const_cast<WindowWin32*>(this), newSize);
	}
	if(oldPos != newPos) {
		change = true;
		onMove.Broadcast(const_cast<WindowWin32*>(this), newPos);
	}

	return change;
}

bool WindowWin32::UpdateInnerRect()
{
	RECT r;
	GetClientRect(m_Window, &r);
	m_InnerRect.left = (float)r.left;
	m_InnerRect.top = (float)r.top;
	m_InnerRect.right = (float)r.right;
	m_InnerRect.bottom = (float)r.bottom;

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

bool WindowWin32::Present(video::Image* image, const math::RectI& SourceRect, const math::RectI& DestRect)
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

	math::RectI DstRect = DestRect;
	math::RectI WinRect = math::RectI(rect.left, rect.top, rect.right, rect.bottom);
	if(DestRect.IsEmpty())
		DstRect = WinRect;
	else
		DstRect.FitInto(WinRect);

	math::RectI SrcRect = SourceRect;
	math::RectI ImageRect = math::RectI(0, 0, image->GetSize().width, image->GetSize().height);
	if(SrcRect.IsEmpty())
		SrcRect = ImageRect;
	else
		SrcRect.FitInto(ImageRect);

	math::Dimension2<int> ImageDim = ImageRect.GetSize();
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
		DstRect.left, DstRect.top, DstRect.GetWidth(), DstRect.GetHeight(),
		SrcRect.left, SrcRect.top, SrcRect.GetWidth(), SrcRect.GetHeight(),
		mem,
		(const BITMAPINFO*)&bi, DIB_RGB_COLORS, SRCCOPY);

	LUX_FREE_ARRAY(data);

	image->Unlock();
	ReleaseDC(m_Window, DC);

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

Cursor* WindowWin32::GetCursor() const
{
	return m_Cursor;
}

core::Name WindowWin32::GetReferableType() const
{
	static const core::Name name = "lux.gui.SystemWindow";
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
			math::Vector2F pos = m_Cursor->GetGrabbingPosition();
			if(p.x != pos.x || p.y != pos.y)
				m_Cursor->SetPosition(pos.x, pos.y);

			result = 0;
			return true;
		} else {
			m_Cursor->Tick();
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
