#ifdef LUX_WINDOWS
#include "CursorWin32.h"
#include "StrippedWindows.h"

namespace lux
{
namespace gui
{

CursorWin32::CursorWin32(Window* window) : m_Window(window), m_Grabbing(false)
{
	m_OldPos = math::Vector2F(-1, -1);

	m_Window->onResize.Connect(this, &CursorWin32::OnResize);
	OnResize(m_Window, m_Window->GetFinalSize());
	CURSORINFO Info;
	Info.cbSize = sizeof(CURSORINFO);
	BOOL GotInfo = GetCursorInfo(&Info);
	if(GotInfo)
		m_IsVisible = (Info.flags == CURSOR_SHOWING);
}

CursorWin32::~CursorWin32()
{
	if(m_Window)
		m_Window->onResize.DisconnectClass(this);
}

void CursorWin32::SetState(ECursorState state)
{
	if(state != m_State) {
		HCURSOR cursor = NULL;
		switch(state) {
		case ECursorState::Normal:
			cursor = LoadCursorW(NULL, IDC_ARROW);
			break;
		case ECursorState::Beam:
			cursor = LoadCursorW(NULL, IDC_IBEAM);
			break;
		case ECursorState::Wait:
			cursor = LoadCursorW(NULL, IDC_WAIT);
			break;
		}
		if(cursor)
			SetCursor(cursor);
	}

	m_State = state;
}

ECursorState CursorWin32::GetState() const
{
	return m_State;
}

void CursorWin32::SetPosition(float x, float y)
{
	// Nur wenn das Fenster aktiv ist
	if(m_Window->IsEnabled()) {
		POINT p = {(LONG)x, (LONG)y};
		ClientToScreen((HWND)m_Window->GetDeviceWindow(), &p);
		SetCursorPos(p.x, p.y);
	}
}

math::Vector2F CursorWin32::GetPosition() const
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient((HWND)m_Window->GetDeviceWindow(), &p);

	return math::Vector2F((float)p.x, (float)p.y);
}

void CursorWin32::SetRelPosition(float x, float y)
{
	SetPosition(x * m_WindowSize.width, y * m_WindowSize.height);
}

math::Vector2F CursorWin32::GetRelPosition() const
{
	math::Vector2F AbsPos = GetPosition();

	return math::Vector2F(AbsPos.x * m_InvWindowSize.width, AbsPos.y * m_InvWindowSize.height);
}

const math::Dimension2F& CursorWin32::GetScreenSize() const
{
	return m_WindowSize;
}

void CursorWin32::SetVisible(bool Visible)
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

bool CursorWin32::IsVisible() const
{
	return m_IsVisible;
}

bool CursorWin32::IsGrabbing() const
{
	return m_Grabbing;
}

void CursorWin32::SetGrabbing(bool Grab)
{
	if(Grab && !m_Grabbing)
		m_GrabbingPosition = GetPosition();
	m_Grabbing = Grab;
}

const math::Vector2F& CursorWin32::GetGrabbingPosition() const
{
	return m_GrabbingPosition;
}

void CursorWin32::OnResize(Window* window, const math::Dimension2F& newSize)
{
	// Should never happen, but isn't a problem
	lxAssert(m_Window == window);
	LUX_UNUSED(window);

	m_WindowSize = newSize;
	m_InvWindowSize.width = 1.0f / m_WindowSize.width;
	m_InvWindowSize.height = 1.0f / m_WindowSize.height;
}

void CursorWin32::Tick()
{
	if(!m_Grabbing) {
		auto pos = GetPosition();
		if(m_OldPos != pos) {
			onCursorMove.Broadcast(math::Vector2F(pos));
			m_OldPos = pos;
		}
	}
}

}
}
#endif // LUX_WINDOWS
