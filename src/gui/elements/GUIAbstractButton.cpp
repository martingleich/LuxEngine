#include "gui/elements/GUIAbstractButton.h"
#include "gui/GUIEnvironment.h"

namespace lux
{
namespace gui
{

AbstractButton::AbstractButton(bool isSwitch) :
	m_IsPressed(false),
	m_IsPushButton(!isSwitch)
{
}

AbstractButton::~AbstractButton()
{
}

bool AbstractButton::OnMouseEvent(const gui::MouseEvent& e)
{
	if(e.type == gui::MouseEvent::LDown && IsPointInside(e.pos)) {
		if(IsFocusable())
			m_Environment->SetFocused(this);
		if(m_IsPushButton)
			m_IsPressed = true;
		else {
			m_IsPressed = !m_IsPressed;
			onClick.Broadcast(this);
		}
		return true;
	}
	if(e.type == gui::MouseEvent::LUp) {
		if(m_IsPushButton) {
			m_IsPressed = false;
			onClick.Broadcast(this);
		}
		return true;
	}
	return false;
}

bool AbstractButton::OnKeyboardEvent(const gui::KeyboardEvent& e)
{
	if(e.key == input::KEY_RETURN && IsFocused()) {
		if(e.down) {
			if(m_IsPushButton) {
				m_IsPressed = true;
			} else {
				m_IsPressed = !m_IsPressed;
			}
			onClick.Broadcast(this);
		} else {
			if(m_IsPushButton)
				m_IsPressed = false;
		}
		return true;
	}
	return false;
}

bool AbstractButton::OnElementEvent(const gui::ElementEvent& e)
{
	if(e.type == e.MouseLeave || e.type == e.FocusLost) {
		if(m_IsPushButton)
			m_IsPressed = false;
		return true;
	}

	return false;
}

} // namespace gui
} // namespace lux