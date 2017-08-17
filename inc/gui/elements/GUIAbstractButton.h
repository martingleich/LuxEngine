#ifndef INCLUDED_GUI_ABSTRACT_BUTTON_H
#define INCLUDED_GUI_ABSTRACT_BUTTON_H
#include "gui/GUIElement.h"

namespace lux
{
namespace gui
{

class AbstractButton : public Element
{
public:
	LUX_API AbstractButton();
	LUX_API ~AbstractButton();

	LUX_API bool OnMouseEvent(const gui::MouseEvent& e);
	LUX_API bool OnElementEvent(const gui::ElementEvent& e);

	bool IsPressed() const
	{
		return m_IsPressed;
	}

	event::Signal<Element*> onClick;

protected:
	virtual bool IsRealClick(const math::Vector2F& pos)
	{
		LUX_UNUSED(pos);
		return true;
	}

protected:
	bool m_IsPressed;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_ABSTRACT_BUTTON_H