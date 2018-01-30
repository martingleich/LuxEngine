#ifndef INCLUDED_GUI_ABSTRACT_BUTTON_H
#define INCLUDED_GUI_ABSTRACT_BUTTON_H
#include "gui/GUIElement.h"
#include "events/lxSignal.h"

namespace lux
{
namespace gui
{

class AbstractButton : public Element
{
public:
	LUX_API AbstractButton(bool clickOnRelease);
	LUX_API ~AbstractButton();

	LUX_API bool OnMouseEvent(const gui::MouseEvent& e);
	LUX_API bool OnKeyboardEvent(const gui::KeyboardEvent& e);
	LUX_API bool OnElementEvent(const gui::ElementEvent& e);

	LUX_API bool IsPressed();
	event::Signal<> onClick;

protected:
	LUX_API virtual void OnClick();

protected:
	bool m_IsPressed;
	bool m_ClickOnRelease;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_ABSTRACT_BUTTON_H