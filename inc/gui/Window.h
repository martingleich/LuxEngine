#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H
#include "gui/GUIElement.h"

namespace lux
{
namespace video
{
class Image;
}

namespace gui
{
//! Represent a single window
class Window : public Element
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

	//! Maximize the window
	/**
	\return True if the window was maximized,
	false if an error occured or the window aleardy is maximized

	\ref IsMaximized
	*/
	virtual bool Maximize() = 0;
	virtual bool IsMaximized() const = 0;

	//! Maximize the window
	/**
	\return True if the window was minimized,
	false if an error occured or the window aleardy is minimized

	\ref IsMinimized
	*/
	virtual bool Minimize() = 0;
	virtual bool IsMinimized() const = 0;

	virtual bool IsActive() const = 0;

	//! Allow the user to resize the window
	/**
	\param Resize Is the user allowed to resize the window
	\return Was the state set successfully
	*/
	virtual bool SetResizable(bool Resize) = 0;

	//! Set the window to fullscreen
	/**
	The window uses the current size in fullscreen mode
	\param fullscreen Should the window enter or leave fullscreen
	\return Was the state set successfully

	\ref IsFullscreen
	*/
	virtual bool SetFullscreen(bool fullscreen) = 0;
	virtual bool IsFullscreen() const = 0;

	//! Restore the state of the window
	virtual void Restore() = 0;

	//! Close the window
	virtual void Close() = 0;

	//! Present an image to the window area
	/**
	The image is scaled between SourceRect and DestRect
	\param image The image to draw to the window
	\param SourceRect The part of the image to use, math::RectI::EMPTY to draw the full image
	\param DestRect Where should the image be drawn, math::RectI::EMPTY to fill to full window
	\return Was the image successfully drawn
	*/
	virtual bool Present(video::Image* image, const math::RectI& SourceRect = math::RectI::EMPTY, const math::RectI& DestRect = math::RectI::EMPTY) = 0;

	//! Get access to the device depended window
	/**
	See the specific window implementation for more information
	The return value is always NULL, if no window is available
	\return The device window
	*/
	virtual void* GetDeviceWindow() const = 0;

	event::Signal<Window*, const math::Dimension2F&> onResize;
	event::Signal<Window*, const math::Vector2F&> onMove;
	event::Signal<Window*, EStateChange> onStateChange;
	event::Signal<Window*> onClose;
};

} // namespace gui
} // namespace lux

#endif