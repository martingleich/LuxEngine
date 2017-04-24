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
	math::vector2i GetPosition() const;
	math::vector2f GetRelPosition() const;
	const math::dimension2du& GetScreenSize() const;
	void SetVisible(bool Visible);
	bool IsVisible() const;
	bool IsGrabbing() const;
	void SetGrabbing(bool Grab);
	void Tick();
	void OnResize(Window& window, const math::dimension2du& newSize);

	const math::vector2i& GetGrabbingPosition() const
	{
		return m_GrabbingPosition;
	}

private:
	void UpdateSize();

private:
	bool m_IsVisible;
	WeakRef<Window> m_Window;
	math::dimension2du m_WindowSize;
	math::dimension2df m_InvWindowSize;

	bool m_Grabbing;
	math::vector2i m_GrabbingPosition;
};

}
}

#endif // LUX_WINDOWS

#endif // !INCLUDED_CURSORCONTROLWIN32_H
