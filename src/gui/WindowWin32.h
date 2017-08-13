#ifndef INCLUDED_WINDOW_WIN32_H
#define INCLUDED_WINDOW_WIN32_H
#include "WindowBase.h"
#include "gui/CursorWin32.h"

#ifdef LUX_WINDOWS
#include "StrippedWindows.h"

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
	void SetText(const String& text);

	void SetInnerSize(const math::Dimension2<ScalarDistanceF>& size);

	bool Maximize();
	bool Minimize();
	void Restore();

	bool SetResizable(bool resize);
	void Close();
	bool Present(video::Image* image, const math::RectI& SourceRect = math::RectI::EMPTY, const math::RectI& DestRect = math::RectI::EMPTY);

	// Cast the return value the HWND to get the device window
	void* GetDeviceWindow() const;

	Cursor* GetCursor() const;
	bool HandleMessages(UINT Message,
		WPARAM WParam,
		LPARAM LParam,
		LRESULT& result);

	void SetDirtyRect();
	math::RectF GetParentInnerRect() const;
	void UpdateFinalRect() const;
	void UpdateInnerRect() const;

	core::Name GetReferableType() const;

private:
	HWND m_Window;
	bool m_IsFullscreen;

	SavedWindow m_SavedWindow;

	StrongRef<CursorWin32> m_Cursor;
};

} // namespace gui
} // namespace lux

#endif // LUX_WINDOWS


#endif // !INCLUDED_WINDOWWIN32_H
