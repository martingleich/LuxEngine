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
class WindowWin32MsgCallback
{
public:
	virtual ~WindowWin32MsgCallback() {}

	virtual bool OnMsg(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& outResult) = 0;
};

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
	WindowWin32MsgCallback* m_MsgCallback;

public:
	WindowWin32(HWND window, WindowWin32MsgCallback* msgCallback);
	WindowWin32(const math::Dimension2U& size, const String& title, WindowWin32MsgCallback* msgCallbac);

	HWND CreateNewWindow(const math::Dimension2U& size, const String& title);
	void Init(HWND window);

	~WindowWin32();

	bool SwitchFullscreen(bool Fullscreen);
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

	bool RunMessageQueue();

	bool HandleMessages(UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);
};

}
}

#endif // LUX_WINDOWS


#endif // !INCLUDED_WINDOWWIN32_H
