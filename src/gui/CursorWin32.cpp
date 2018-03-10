#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "CursorWin32.h"
#include "platform/StrippedWindows.h"

namespace lux
{
namespace gui
{

CursorWin32::CursorWin32(Window* window) : 
	m_Window(window),
	m_Grabbing(false),
	m_Visible(true)
{
	m_OldPos = math::Vector2F(-1, -1);

	m_Window->onResize.Connect(this, &CursorWin32::OnResize);
	OnResize(m_Window, m_Window->GetFinalSize());
}

CursorWin32::~CursorWin32()
{
	if(m_Window)
		m_Window->onResize.DisconnectClass(this);
}

void CursorWin32::SetState(ECursorState state)
{
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

void CursorWin32::SetVisible(bool visible)
{
	m_Visible = visible;
}

bool CursorWin32::IsVisible() const
{
	return m_Visible;
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
