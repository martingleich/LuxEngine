#ifndef INCLUDED_LUX_CURSOR_WIN32_H
#define INCLUDED_LUX_CURSOR_WIN32_H
#include "gui/Cursor.h"
#include "gui/Window.h"

#ifdef LUX_WINDOWS
#include "platform/StrippedWindows.h"

namespace lux
{
namespace gui
{

class WindowWin32;
class CursorWin32 : public Cursor
{
	friend class WindowWin32;
private:
	struct CursorConfineMode : core::Uncopyable
	{
		CursorConfineMode()
		{
		}
		bool Clip(RECT clip)
		{
			BOOL b = GetClipCursor(&oldClip);
			if(!b)
				return false;
			b = ClipCursor(&clip);
			if(!b)
				return false;
			clipped = true;
			return true;
		}
		void Unclip()
		{
			if(clipped)
				ClipCursor(&oldClip);
			clipped = false;
		}
		~CursorConfineMode()
		{
			Unclip();
		}
		RECT oldClip;
		bool clipped;
	};
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
	void GrabCursor(const math::Vector2F& pos);
	void GrabCursor();
	void UnGrabCursor(const math::Vector2F& pos);
	void UnGrabCursor();

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
	CursorConfineMode m_ConfineMode;
};

} // namespace gui
} // namespace lux

#endif // LUX_WINDOWS

#endif // !INCLUDED_LUX_CURSORCONTROLWIN32_H
