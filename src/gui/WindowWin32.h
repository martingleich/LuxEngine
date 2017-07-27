#ifndef INCLUDED_WINDOW_WIN32_H
#define INCLUDED_WINDOW_WIN32_H
#include "WindowBase.h"

#ifdef LUX_WINDOWS
#include "StrippedWindows.h"

namespace lux
{
namespace gui
{
class CursorControl;

class WindowWin32 : public WindowBase
{
private:
	HWND m_Window;
	bool m_IsFullscreen;    // The current fullscreen state of the window, changed by system

	struct SavedWindow
	{
		bool IsMaxed;
		LONG Style;
		LONG ExStyle;
		RECT WinRect;
	};
	SavedWindow m_SavedWindow;

	StrongRef<CursorControl> m_CursorControl;
public:
	WindowWin32();
	~WindowWin32();

	bool SwitchFullscreen(bool Fullscreen);
	bool Init(HWND Window);
	void SetTitle(const String& title);
	void SetSize(const math::Dimension2U& Size);
	void SetPosition(const math::Vector2I& Position);
	bool Maximize();
	bool Minimize();
	bool SetResizable(bool Resize);
	bool Close();
	bool Present(video::Image* image, const math::RectI& SourceRect = math::RectI::EMPTY, const math::RectI& DestRect = math::RectI::EMPTY);
	bool Restore();

	// Cast the return value the HWND to get the device window
	void* GetDeviceWindow() const;

	WeakRef<CursorControl> GetCursorControl();

	void Tick();

	bool HandleMessages(UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);
};

}

}

#endif // LUX_WINDOWS


#endif // !INCLUDED_WINDOWWIN32_H
