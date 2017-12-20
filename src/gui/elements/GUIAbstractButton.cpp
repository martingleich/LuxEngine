#include "gui/elements/GUIAbstractButton.h"
#include "gui/GUIEnvironment.h"

namespace lux
{
namespace gui
{

AbstractButton::AbstractButton(bool clickOnRelease) :
	m_IsPressed(false),
	m_ClickOnRelease(clickOnRelease)
{
}

AbstractButton::~AbstractButton()
{
}

bool AbstractButton::OnMouseEvent(const gui::MouseEvent& e)
{
	if(e.type == gui::MouseEvent::LDown && IsPointInside(e.pos)) {
		m_IsPressed = true;
		if(!m_ClickOnRelease)
			OnClick();
		return true;
	}
	if(e.type == gui::MouseEvent::LUp) {
		m_IsPressed = false;
		if(m_ClickOnRelease)
			OnClick();
		return true;
	}
	return false;
}

bool AbstractButton::OnKeyboardEvent(const gui::KeyboardEvent& e)
{
	if(e.key == input::KEY_RETURN) {
		if(e.down) {
			m_IsPressed = true;
			if(!m_ClickOnRelease)
				OnClick();
		} else {
			if(m_ClickOnRelease) {
				m_IsPressed = false;
				OnClick();
			}
		}
		return true;
	}
	return false;
}

bool AbstractButton::OnElementEvent(const gui::ElementEvent& e)
{
	if(e.type == e.MouseLeave || e.type == e.FocusLost) {
		m_IsPressed = false;
		return true;
	}

	return false;
}

bool AbstractButton::IsPressed()
{
	return m_IsPressed;
}

void AbstractButton::OnClick()
{
	onClick.Broadcast();
}

} // namespace gui
} // namespace lux