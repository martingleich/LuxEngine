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
		if(m_IsPushButton)
			m_IsPressed = true;
		else {
			m_IsPressed = !m_IsPressed;
			onClick.Broadcast();
		}
		return true;
	}
	if(e.type == gui::MouseEvent::LUp) {
		if(m_IsPushButton) {
			m_IsPressed = false;
			onClick.Broadcast();
		}
		return true;
	}
	return false;
}

bool AbstractButton::OnKeyboardEvent(const gui::KeyboardEvent& e)
{
	if(e.key == input::KEY_RETURN) {
		if(e.down) {
			if(m_IsPushButton) {
				m_IsPressed = true;
			} else {
				m_IsPressed = !m_IsPressed;
			}
			onClick.Broadcast();
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

EGUIState AbstractButton::GetState() const
{
	EGUIState state = Element::GetState();
	if(m_IsPressed)
		state |= EGUIState::Sunken;
	return state;
}

} // namespace gui
} // namespace lux