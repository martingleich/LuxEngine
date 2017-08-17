#include "gui/elements/GUIAbstractButton.h"
#include "gui/GUIEnvironment.h"

namespace lux
{
namespace gui
{

AbstractButton::AbstractButton() :
	m_IsPressed(false)
{
}

AbstractButton::~AbstractButton()
{
}

bool AbstractButton::OnMouseEvent(const gui::MouseEvent& e)
{
	if(e.type == gui::MouseEvent::LDown && IsRealClick(e.pos)) {
		m_IsPressed = true;
		return true;
	}
	if(e.type == gui::MouseEvent::LUp) {
		m_IsPressed = false;
		onClick.Broadcast(this);
		return true;
	}
	return false;
}

bool AbstractButton::OnElementEvent(const gui::ElementEvent& e)
{
	if(e.type == e.MouseLeave) {
		m_IsPressed = false;
		return true;
	}

	return false;
}

} // namespace gui
} // namespace lux