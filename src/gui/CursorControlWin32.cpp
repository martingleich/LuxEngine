#ifdef LUX_WINDOWS
#include "CursorControlWin32.h"
#include "StrippedWindows.h"

namespace lux
{
namespace gui
{

CursorControlWin32::CursorControlWin32(Window* Window) : m_Window(Window), m_Grabbing(false)
{
	m_Window->RegisterCallback(this);
	UpdateSize();
	CURSORINFO Info;
	Info.cbSize = sizeof(CURSORINFO);
	BOOL GotInfo = GetCursorInfo(&Info);
	if(GotInfo)
		m_IsVisible = (Info.flags == CURSOR_SHOWING);
}

CursorControlWin32::~CursorControlWin32()
{
	m_Window->UnregisterCallback(this);
}

void CursorControlWin32::UpdateSize()
{
	m_WindowSize = m_Window->GetSize();
	m_InvWindowSize.width = 1.0f / m_WindowSize.width;
	m_InvWindowSize.height = 1.0f / m_WindowSize.height;
}

void CursorControlWin32::SetPosition(int x, int y)
{
	// Nur wenn das Fenster aktiv ist
	if(m_Window->IsActive()) {
		POINT p;
		p.x = x;
		p.y = y;
		ClientToScreen((HWND)m_Window->GetDeviceWindow(), &p);
		SetCursorPos(p.x, p.y);
	}
}

void CursorControlWin32::SetRelPosition(float x, float y)
{
	SetPosition((int)(x * m_WindowSize.width), (int)(y * m_WindowSize.height));
}

math::vector2i CursorControlWin32::GetPosition() const
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient((HWND)m_Window->GetDeviceWindow(), &p);

	return math::vector2i(p.x, p.y);
}

math::vector2f CursorControlWin32::GetRelPosition() const
{
	math::vector2i AbsPos = GetPosition();

	return math::vector2f(AbsPos.x * m_InvWindowSize.width, AbsPos.y * m_InvWindowSize.height);
}

const math::dimension2du& CursorControlWin32::GetScreenSize() const
{
	return m_WindowSize;
}

void CursorControlWin32::SetVisible(bool Visible)
{
	CURSORINFO Info;
	Info.cbSize = sizeof(CURSORINFO);
	BOOL GotInfo = GetCursorInfo(&Info);
	if(GotInfo) {
		// Ist der Cursor schon sichtbar/unsichtbar
		if(!((Visible && Info.flags == CURSOR_SHOWING)
			|| (!Visible && Info.flags == 0))) {
			// ShowCursor verringert nur einen Refernzzähler, erst abbrechen wenn er 0 erreicht
			int ShowResult = ShowCursor(Visible ? TRUE : FALSE);
			while(Visible != (ShowResult >= 0)) {
				ShowResult = ShowCursor(Visible ? TRUE : FALSE);
			}
		}
	}

	m_IsVisible = Visible;
}

bool CursorControlWin32::IsVisible() const
{
	return m_IsVisible;
}

bool CursorControlWin32::IsGrabbing() const
{
	return m_Grabbing;
}

void CursorControlWin32::SetGrabbing(bool Grab)
{
	if(Grab && !m_Grabbing)
		m_GrabbingPosition = GetPosition();
	m_Grabbing = Grab;
}

void CursorControlWin32::Tick()
{
	if(m_Grabbing && m_Window->IsActive() && m_Window->IsFocused())
		SetPosition(m_GrabbingPosition.x, m_GrabbingPosition.y);
}

void CursorControlWin32::OnResize(Window& window, const math::dimension2du& newSize)
{
	// Should never happen, but isn't a problem
	lxAssert(m_Window == &window);
	
	LUX_UNUSED(window);
	LUX_UNUSED(newSize); // The size is read directly from the window.
	UpdateSize();
}

}
}
#endif // LUX_WINDOWS
