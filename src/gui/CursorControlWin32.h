#ifndef INCLUDED_CURSORCONTROL_WIN32_H
#define INCLUDED_CURSORCONTROL_WIN32_H
#include "gui/CursorControl.h"
#include "gui/Window.h"

#ifdef LUX_WINDOWS

namespace lux
{
namespace gui
{

class CursorControlWin32 : public CursorControl, public WindowEventCallback
{
public:
	CursorControlWin32(Window* window);
	~CursorControlWin32();
	void SetPosition(int x, int y);
	void SetRelPosition(float x, float y);
	math::Vector2I GetPosition() const;
	math::Vector2F GetRelPosition() const;
	const math::Dimension2U& GetScreenSize() const;
	void SetVisible(bool Visible);
	bool IsVisible() const;
	bool IsGrabbing() const;
	void SetGrabbing(bool Grab);
	void Tick();
	void OnResize(Window& window, const math::Dimension2U& newSize);

	const math::Vector2I& GetGrabbingPosition() const
	{
		return m_GrabbingPosition;
	}

private:
	void UpdateSize();

private:
	bool m_IsVisible;
	WeakRef<Window> m_Window;
	math::Dimension2U m_WindowSize;
	math::Dimension2F m_InvWindowSize;

	bool m_Grabbing;
	math::Vector2I m_GrabbingPosition;
};

}
}

#endif // LUX_WINDOWS

#endif // !INCLUDED_CURSORCONTROLWIN32_H
