#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "CursorWin32.h"

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

void CursorWin32::GrabCursor(const math::Vector2F& pos)
{
	if(m_Grabbing)
		return;

	POINT p = {(LONG)pos.x, (LONG)pos.y};
	ClientToScreen((HWND)m_Window->GetDeviceWindow(), &p);
	m_GrabbingPosition = math::Vector2F((float)p.x, (float)p.y);
	RECT clip;
	clip.left = (LONG)p.x;
	clip.right= (LONG)p.x;
	clip.top = (LONG)p.y;
	clip.bottom= (LONG)p.y;
	m_ConfineMode.Clip(clip);
	m_Grabbing = true;
}

void CursorWin32::GrabCursor()
{
	if(m_Grabbing)
		return;
	POINT p;
	POINT cp;
	GetCursorPos(&p);
	cp = p;
	ScreenToClient((HWND)m_Window->GetDeviceWindow(), &cp);
	m_GrabbingPosition = math::Vector2F((float)cp.x, (float)cp.y);
	RECT clip;
	clip.left = p.x;
	clip.right= p.x;
	clip.top = p.y;
	clip.bottom= p.y;
	m_ConfineMode.Clip(clip);
	m_Grabbing = true;
}

void CursorWin32::UnGrabCursor(const math::Vector2F& pos)
{
	if(!m_Grabbing)
		return;
	m_ConfineMode.Unclip();
	SetPosition(pos.x, pos.y);
	m_Grabbing = false;
}
void CursorWin32::UnGrabCursor()
{
	if(!m_Grabbing)
		return;
	m_ConfineMode.Unclip();
	m_Grabbing = false;
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
