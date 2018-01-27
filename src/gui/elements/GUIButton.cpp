#include "gui/elements/GUIButton.h"
#include "gui/GUISkin.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::Button, "lux.gui.Button");

namespace lux
{
namespace gui
{
Button::Button() :
	AbstractButton(true)
{
	SetAlignment(EAlign::Centered);
}

Button::~Button()
{
}

void Button::Paint(Renderer* r)
{
	auto skin = GetSkin();
	if(!skin)
		return;
	auto palette = GetFinalPalette();
	math::Vector2F textOffset;

	if(m_IsPressed)
		textOffset = skin->GetPropertyVector("lux.gui.sunkenOffset");

	auto state = GetState();
	PaintOptions options;
	options.palette = palette;
	skin->DrawControl(r, this, GetFinalRect(), EGUIControl::Button, state, options);

	FontRenderSettings settings;
	settings.color = palette.GetWindowText(GetState());
	m_Text.Render(
		r, GetFont(),
		settings, false, GetAlignment(),
		GetFinalInnerRect() + textOffset);
}

void Button::SetText(const core::String& text)
{
	m_Text.SetText(text);
}

const core::String& Button::GetText() const
{
	return m_Text.GetText();
}

EGUIState Button::GetState() const
{
	EGUIState state = Element::GetState();
	if(m_IsPressed)
		state |= EGUIState::Sunken;
	return state;
}

} // namespace gui
} // namespace lux