#include "gui/elements/GUIButton.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

static ::lux::Referable* PushButtonInternalCreatorFunc(const void*) { return LUX_NEW(::lux::gui::Button)(false); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock PushButtonInternalReferableRegisterStaticObject("lux.gui.Button", &PushButtonInternalCreatorFunc);

static ::lux::Referable* SwitchButtonInternalCreatorFunc(const void*) { return LUX_NEW(::lux::gui::Button)(true); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock SwitchButtonInternalReferableRegisterStaticObject("lux.gui.SwitchButton", &SwitchButtonInternalCreatorFunc);

namespace lux
{
namespace gui
{
Button::Button(bool isSwitchButton) :
	AbstractButton(isSwitchButton),
	m_Color(video::Color::LightGray)
{
}

Button::~Button()
{
}

void Button::Paint(Renderer* r)
{
	auto skin = GetSkin();
	if(!skin)
		return;
	video::Color textColor;
	math::Vector2F textOffset;

	if(m_IsPressed) {
		skin->DrawPrimitive(r, this->GetState(), EGUIPrimitive::ButtonPressed, this->GetFinalRect(), m_Color);
		textColor = video::Color::Black;
		textOffset = skin->buttonPressedTextOffset;
	} else {
		skin->DrawPrimitive(r, this->GetState(), EGUIPrimitive::Button, this->GetFinalRect(), m_Color);
		textColor = video::Color::Black;
	}
	r->DrawText(skin->defaultFont, m_Text, GetFinalRect().GetCenter() + textOffset, gui::Font::EAlign::Centered, textColor, nullptr);
}

void Button::SetColor(video::Color c)
{
	m_Color = c;
}

video::Color Button::GetColor() const
{
	return m_Color;
}

core::Name Button::GetReferableType() const
{
	static const core::Name name = "lux.gui.Button";
	return name;
}

} // namespace gui
} // namespace lux