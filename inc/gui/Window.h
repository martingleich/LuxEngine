#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "math/rect.h"

namespace lux
{
namespace video
{
class Image;
}

namespace gui
{
class CursorControl;
class WindowEventCallback;

//! Represent a single window
class Window : public ReferenceCounted
{
public:
	//! The statechanges a window can do
	enum class EStateChange
	{
		Minimize,       //!< The window was minimized
		Maximize,       //!< The window was maximized
		Fullscreen,     //!< The window went to fullscreen
		FocusLost,      //!< Thes window lost the focus
		FocusGained,    //!< The window gained the focus
		Normal,         //!< The window returned to normal size
		Activated,      //!< The window was activated
		Deactivated     //!< The window was deactivated
	};

public:
	virtual ~Window()
	{
	}

	//! Register for window callbacks
	/**
	You are informed when window parameters are changing
	\param call The Callback object

	\ref UnregisterCallback
	*/
	virtual void RegisterCallback(WindowEventCallback* call) = 0;

	//! Unregister for window callbacks
	/**
	You are no longer informed when window parameters are changing
	\param call The Callback object

	\ref RegisterCallback
	*/
	virtual void UnregisterCallback(WindowEventCallback* call) = 0;

	//! Set the title shown by the window
	/**
	\param title The new title of the window

	\ref GetTitle
	*/
	virtual void SetTitle(const String& title) = 0;

	//! Get the current title of the window
	/**
	\return The current title of the window

	\ref SetTitle
	*/
	virtual const String& GetTitle() const = 0;

	//! Set the size of the window in pixel
	/**
	\param Size The new size

	\ref GetSize
	*/
	virtual void SetSize(const math::dimension2du& Size) = 0;

	//! Get the current size of the window in pixel
	/**
	\return The current window size

	\ref SetSize
	*/
	virtual const math::dimension2du& GetSize() const = 0;

	//! Set the position of the window
	/**
	The position is set in pixel and relative to the parent window
	\param Position The new position of the window

	\ref GetPosition
	*/
	virtual void SetPosition(const math::vector2i& Position) = 0;

	//! Get the current position of the window
	/**
	The position is given in pixel and relative to the parent window
	\return The current position of the window

	\ref SetPosition
	*/
	virtual const math::vector2i& GetPosition() const = 0;

	//! Maximize the window
	/**
	\return True if the window was maximized,
	false if an error occured or the window aleardy is maximized

	\ref IsMaximized
	*/
	virtual bool Maximize() = 0;

	//! Maximize the window
	/**
	\return True if the window was minimized,
	false if an error occured or the window aleardy is minimized

	\ref IsMinimized
	*/
	virtual bool Minimize() = 0;

	//! Allow the user to resize the window
	/**
	\param Resize Is the user allowed to resize the window
	\return Was the state set successfully
	*/
	virtual bool SetResizable(bool Resize) = 0;

	//! Set the window to fullscreen
	/**
	The window uses the current size in fullscreen mode
	\param Fullscreen Should the window enter or leave fullscreen
	\return Was the state set successfully

	\ref IsFullscreen
	*/
	virtual bool SetFullscreen(bool Fullscreen) = 0;

	//! Is the window minimized
	/**
	\return Is the window minized

	\ref Minimize
	*/
	virtual bool IsMinimized() const = 0;

	//! Is the window maximized
	/**
	\return Is the window maximized

	\ref Maximize
	*/
	virtual bool IsMaximized() const = 0;
	//! Is the window focused
	/**
	\return Is the window focused
	*/
	virtual bool IsFocused() const = 0;
	//! Is the window active
	/**
	\return Is the window active
	*/
	virtual bool IsActive() const = 0;
	//! Is the window in fullscreen mode
	/**
	\return Is the window in fullscreen mode

	\ref SetFullscreen
	*/
	virtual bool IsFullscreen() const = 0;

	//! Close the window
	/**
	\return Was the window successfully closed
	*/
	virtual bool Close() = 0;

	//! Present an image to the window area
	/**
	The image is scaled between SourceRect and DestRect
	\param image The image to draw to the window
	\param SourceRect The part of the image to use, math::recti::EMPTY to draw the full image
	\param DestRect Where should the image be drawn, math::recti::EMPTY to fill to full window
	\return Was the image successfully drawn
	*/
	virtual bool Present(video::Image* image, const math::recti& SourceRect = math::recti::EMPTY, const math::recti& DestRect = math::recti::EMPTY) = 0;

	//! Restore the state of the window
	/**
	Show the window normal, no fullscreen, not minimized, not deactivated...
	\return Was the window restored
	*/
	virtual bool Restore() = 0;

	//! Get access to the device depended window
	/**
	See the specific window implementation for more information
	The return value is always NULL, if no window is available
	\return The device window
	*/
	virtual void* GetDeviceWindow() const = 0;

	//! Get the control over the cursor in the window
	/**
	\return The cursor control, NULL if not available
	*/
	virtual WeakRef<CursorControl> GetCursorControl() = 0;
};

//! The window callback interface
/**
Implement this an hand it to Window::RegisterCallback
to retrieve window state changes
*/
class WindowEventCallback
{
	friend class WindowBase;
private:
	WindowEventCallback* m_Next;
	WindowEventCallback* m_Prev;
	WeakRef<Window> m_Window;

public:
	WindowEventCallback() :
		m_Next(nullptr),
		m_Prev(nullptr),
		m_Window(nullptr)
	{
	}

	virtual ~WindowEventCallback()
	{
		UnregisterCallback();
	}

	//! Call this to remove this receiver from the event queue
	/**
	You will no longer receive event callbacks
	*/
	void UnregisterCallback()
	{
		if(m_Window)
			m_Window->UnregisterCallback(this);
	}

	//! Called when the size of the window changes
	/**
	Is called on window creation too
	\param window The window which changed
	\param newSize The new size of the window

	\ref Window::GetSize
	*/
	virtual void OnResize(Window& window, const math::dimension2du& newSize)
	{
		LUX_UNUSED(window);
		LUX_UNUSED(newSize);
	}

	//! Called when the window was moved
	/**
	\param window The window which changed
	\param NewPos The new position of the window

	\ref Window::GetPosition
	*/
	virtual void OnMove(Window& window, const math::vector2i& NewPos)
	{
		LUX_UNUSED(window);
		LUX_UNUSED(NewPos);
	}

	//! Called when the presentation of the window changed
	/**
	\param window The window which changed
	\paran NewState The new state of the window
	*/
	virtual void OnStateChanged(Window& window, Window::EStateChange NewState)
	{
		LUX_UNUSED(window);
		LUX_UNUSED(NewState);
	}

	//! Called on attempt to close the window
	/**
	\param window The window which should be closed
	\return Return true, to allow closing the window, false to forbid

	\ref OnClose
	*/
	virtual bool OnClosing(Window& window)
	{
		LUX_UNUSED(window);
		return true;
	}

	//! Called when the window is closed
	/**
	Called after OnClosing, when it returned true
	\param window The window which was closed

	\ref OnClosing
	*/
	virtual void OnClose(Window& window)
	{
		LUX_UNUSED(window);
	}
};

}
}

#endif