#ifndef INCLUDED_WINDOWWIN32_H
#define INCLUDED_WINDOWWIN32_H
#include "WindowBase.h"
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
	void SetTitle(const string& title);
	void SetSize(const math::dimension2du& Size);
	void SetPosition(const math::vector2i& Position);
	bool Maximize();
	bool Minimize();
	bool SetResizable(bool Resize);
	bool Close();
	bool Present(video::Image* image, const math::recti& SourceRect = math::recti::EMPTY, const math::recti& DestRect = math::recti::EMPTY);
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


#endif // !INCLUDED_WINDOWWIN32_H
