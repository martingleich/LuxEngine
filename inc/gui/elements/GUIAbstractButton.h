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
	LUX_API AbstractButton(bool isSwitchButton=false);
	LUX_API ~AbstractButton();

	LUX_API bool OnMouseEvent(const gui::MouseEvent& e);
	LUX_API bool OnKeyboardEvent(const gui::KeyboardEvent& e);
	LUX_API bool OnElementEvent(const gui::ElementEvent& e);

	LUX_API virtual EGUIState GetState() const;

	bool IsPressed() const { return m_IsPressed; }
	bool IsPushButton() const { return m_IsPushButton; }
	
	event::Signal<> onClick;

protected:
	bool m_IsPressed;
	bool m_IsPushButton;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_ABSTRACT_BUTTON_H