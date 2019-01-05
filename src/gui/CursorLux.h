#ifndef INCLUDED_LUX_CURSOR_LUX_H
#define INCLUDED_LUX_CURSOR_LUX_H
#include "gui/Cursor.h"
#include "gui/Window.h"

namespace lux
{
namespace gui
{

class CursorLux : public Cursor
{
public:
	CursorLux(Window* w) :
		m_IsVisible(true),
		m_IsGrabbing(false)
	{
		w->onResize.Connect(this, &CursorLux::OnResize);
		OnResize(w, w->GetFinalSize());
	}

	void Move(const math::Vector2F& v)
	{
		if(!m_IsGrabbing)
			Cursor::SetPosition(m_Position + v*m_Speed);
	}

	void SetState(ECursorState state)
	{
		m_State = state;
	}
	ECursorState GetState() const
	{
		return m_State;
	}
	void SetPosition(float x, float y)
	{
		x = math::Clamp<float>(x, 0, m_ScreenSize.width-1);
		y = math::Clamp<float>(y, 0, m_ScreenSize.height-1);
		if(m_Position.x != x || m_Position.y != y)
			onCursorMove.Broadcast(m_Position);
		
		m_Position.Set(x, y);
	}

	void SetRelPosition(float x, float y)
	{
		SetPosition(x*m_ScreenSize.width, y *m_ScreenSize.height);
	}

	math::Vector2F GetPosition() const
	{
		return m_Position;
	}

	math::Vector2F GetRelPosition() const
	{
		return math::Vector2F(m_Position.x / m_ScreenSize.width, m_Position.y / m_ScreenSize.height);
	}

	const math::Dimension2F& GetScreenSize() const
	{
		return m_ScreenSize;
	}

	void SetVisible(bool visible)
	{
		m_IsVisible = visible;
	}

	bool IsVisible() const
	{
		return m_IsVisible;
	}

	bool IsGrabbing() const
	{
		return m_IsGrabbing;
	}

	void GrabCursor(const math::Vector2F& pos)
	{
		m_IsGrabbing = true;
		m_Position = pos;
	}
	void UnGrabCursor(const math::Vector2F& pos)
	{
		m_Position = pos;
		m_IsGrabbing = false;
	}

private:
	void OnResize(Window* window, const math::Dimension2F& newSize)
	{
		LUX_UNUSED(window);
		m_ScreenSize = newSize;
	}

private:
	ECursorState m_State;
	math::Dimension2F m_ScreenSize;

	float m_Speed = 100.0f;
	math::Vector2F m_Position;

	bool m_IsVisible;
	bool m_IsGrabbing;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_LUX_CURSOR_LUX_H