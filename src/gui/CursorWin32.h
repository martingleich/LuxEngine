#ifndef INCLUDED_CURSOR_WIN32_H
#define INCLUDED_CURSOR_WIN32_H
#include "gui/Cursor.h"
#include "gui/Window.h"

#ifdef LUX_WINDOWS

namespace lux
{
namespace gui
{

class CursorWin32 : public Cursor
{
public:
	CursorWin32(Window* window);
	~CursorWin32();

	void SetState(ECursorState state);
	ECursorState GetState() const;
	void SetPosition(float x, float y);
	math::Vector2F GetPosition() const;

	void SetRelPosition(float x, float y);
	math::Vector2F GetRelPosition() const;

	const math::Dimension2F& GetScreenSize() const;

	void SetVisible(bool Visible);
	bool IsVisible() const;

	bool IsGrabbing() const;
	void SetGrabbing(bool Grab);
	const math::Vector2F& GetGrabbingPosition() const;

	void Tick();

private:
	void OnResize(Window* window, const math::Dimension2F& newSize);

private:
	WeakRef<Window> m_Window;
	math::Dimension2F m_WindowSize;
	math::Dimension2F m_InvWindowSize;

	ECursorState m_State;

	bool m_Grabbing;
	math::Vector2F m_GrabbingPosition;

	math::Vector2F m_OldPos;
	bool m_Visible;
};

} // namespace gui
} // namespace lux

#endif // LUX_WINDOWS

#endif // !INCLUDED_CURSORCONTROLWIN32_H
