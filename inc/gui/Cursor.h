#ifndef INCLUDED_CURSOR_H
#define INCLUDED_CURSOR_H
#include "core/ReferenceCounted.h"
#include "math/Dimension2.h"
#include "core/lxSignal.h"
#include "gui/GUIEnums.h"

namespace lux
{
namespace gui
{

//! Interface to control a cursor
/**
Usable for both Lux GUI Cursors and OS Cursors
*/
class Cursor : public ReferenceCounted
{
public:
	virtual ~Cursor() {}

	virtual void SetState(ECursorState state) = 0;
	virtual ECursorState GetState() const = 0;

	//! Set the absolute positon of the cursor
	/**
	The coordinates are specified in window coordinates and pixel
	\param x The absolute x coordinate
	\param y The absolute y coordinate

	\ref GetPosition
	*/
	virtual void SetPosition(float x, float y) = 0;
	void SetPosition(const math::Vector2F& v)
	{
		SetPosition(v.x, v.y);
	}

	//! Set the relative positon of the mouse
	/**
	The top left corner of the window is 0,0
	The bottom right corner of the window is 1,1
	Bigger coordinates are not allowed
	\param x The relative x coordinate
	\param y The relative y coordiante

	\ref GetRelPositon
	*/
	virtual void SetRelPosition(float x, float y) = 0;
	void SetRelPosition(const math::Vector2F& v)
	{
		SetRelPosition(v.x, v.y);
	}

	//! Retrieve the absolute Positon of the mouse
	/**
	The coords are given in window coordinates an pixel
	\return The absolute positon of the cursor

	\ref SetPosition
	*/
	virtual math::Vector2F GetPosition() const = 0;

	//! Retrieve the relative Positon of the mouse
	/**
	The coords are given in window coordinates an pixel
	The top left corner of the window is 0,0
	The bottom right corner of the window is 1,1
	\return The absolute positon of the cursor

	\ref SetRelPosition
	*/
	virtual math::Vector2F GetRelPosition() const = 0;

	//! Retrieve the size of the screen used by the cursor
	/**
	This is mostly the size of the active window
	\return The size of the screen used by the cursor
	*/
	virtual const math::Dimension2F& GetScreenSize() const = 0;

	//! Set the cursor visibility
	/**
	\param Visible True to reveal the cursor, false to hide

	\ref IsVisible
	*/
	virtual void SetVisible(bool visible) = 0;

	//! Is the cursor currently visible
	/**
	\return The visibility of the cursor

	\ref SetVisible
	*/
	virtual bool IsVisible() const = 0;

	//! Is the cursor in grabbing mode
	/**
	\return Is the cursor currently grabbing

	\ref SetGrabbing
	*/
	virtual bool IsGrabbing() const = 0;

	//! Set the cursor to grabbing mode
	/**
	Grabbing fixes the cursor to one fixed position, but still post mouse move events
	Of course with fixed absolute coordinates
	Manual calls to SetPosition/SetRelPosition are still executed
	\param Grab True to grab the cursor, false to ungrab

	\ref IsGrabbing
	*/
	virtual void SetGrabbing(bool grab) = 0;

	//! Get the current grabbing position
	virtual const math::Vector2F& GetGrabbingPosition() const = 0;

	//! Disables the cursor.
	/**
	The cursor will be made invisible and be placed on the center of the screen, and can't be moved.
	*/
	virtual void Disable()
	{
		this->SetVisible(false);
		this->SetRelPosition(0.5f, 0.5f);
		this->SetGrabbing(true);
	}

	//! Enables the cursor
	/**
	Ungrabs the cursor, and makes it visible, the old position won't be restored.
	*/
	virtual void Enable()
	{
		this->SetVisible(true);
		this->SetGrabbing(false);
	}

	core::Signal<const math::Vector2F&> onCursorMove;
};

} // namespace gui
} // namespace lux

#endif // !INCLUDED_CURSOR_H
