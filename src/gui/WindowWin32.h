#ifndef INCLUDED_LUX_WINDOW_WIN32_H
#define INCLUDED_LUX_WINDOW_WIN32_H
#include "gui/WindowBase.h"
#include "gui/CursorWin32.h"

#ifdef LUX_WINDOWS
#include "platform/StrippedWindows.h"

namespace lux
{
namespace gui
{
class Cursor;

class WindowWin32 : public WindowBase
{
	struct SavedWindow
	{
		bool IsMaxed;
		LONG Style;
		LONG ExStyle;
		RECT WinRect;
	};

public:
	WindowWin32(HWND window);
	~WindowWin32();

	bool SwitchFullscreen(bool fullscreen);
	void SetText(const core::String& text);

	void SetInnerSize(const ScalarDimensionF& size);

	bool Maximize();
	bool Minimize();
	void Restore();

	bool SetResizable(bool resize);
	void Close();
	bool Present(video::Image* image, const math::RectI& SourceRect = math::RectI::EMPTY, const math::RectI& DestRect = math::RectI::EMPTY);

	// Cast the return value the HWND to get the device window
	void* GetDeviceWindow() const;

	Cursor* GetDeviceCursor() const;
	bool HandleMessages(UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

	void Paint(Renderer* r);

	math::RectF GetParentInnerRect() const;

	bool UpdateFinalRect();
	bool UpdateInnerRect();

	core::Name GetReferableType() const;

private:
	void RectChange()
	{
		RECT winRect, clientRect;
		GetWindowRect(m_Window, &winRect);
		GetClientRect(m_Window, &clientRect);
		POINT p = {0, 0};
		ClientToScreen(m_Window, &p);
		clientRect.left += p.x;
		clientRect.right += p.x;
		clientRect.top += p.y;
		clientRect.bottom += p.y;
		auto oldSize = m_Size;
		auto oldPos = m_FinalRect.LeftTop();
		m_WindowScreenRect = winRect;
		m_FinalRect.left = (float)(m_WindowScreenRect.left - clientRect.left);
		m_FinalRect.top = (float)(m_WindowScreenRect.top - clientRect.top);
		m_FinalRect.right = (float)(m_WindowScreenRect.right - clientRect.left);
		m_FinalRect.bottom = (float)(m_WindowScreenRect.bottom - clientRect.top);
		m_InnerRect.left = 0;
		m_InnerRect.top = 0;
		m_InnerRect.right = (float)(clientRect.right - clientRect.left);
		m_InnerRect.bottom = (float)(clientRect.bottom - clientRect.top);
		m_Size.width = m_FinalRect.GetWidth();
		m_Size.height = m_FinalRect.GetHeight();

		if(oldSize != m_Size)
			onResize.Broadcast(this, m_FinalRect.GetSize());
		if(oldPos != m_FinalRect.LeftTop())
			onMove.Broadcast(this, m_FinalRect.LeftTop());
	}

private:
	RECT m_WindowScreenRect;

	HWND m_Window;
	bool m_IsFullscreen;

	SavedWindow m_SavedWindow;

	StrongRef<CursorWin32> m_Cursor;
	HCURSOR m_Beam;
	HCURSOR m_Wait;
	HCURSOR m_Arrow;
};

} // namespace gui
} // namespace lux

#endif // LUX_WINDOWS


#endif // !INCLUDED_LUX_WINDOWWIN32_H
