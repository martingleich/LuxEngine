#include "gui/elements/GUIButton.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

static ::lux::Referable* PushButtonInternalCreatorFunc(const void*) { return LUX_NEW(::lux::gui::Button)(false); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock PushButtonInternalReferableRegisterStaticObject(::lux::core::Name("lux.gui.Button"), &PushButtonInternalCreatorFunc);

static ::lux::Referable* SwitchButtonInternalCreatorFunc(const void*) { return LUX_NEW(::lux::gui::Button)(true); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock SwitchButtonInternalReferableRegisterStaticObject(::lux::core::Name("lux.gui.SwitchButton"), &SwitchButtonInternalCreatorFunc);

namespace lux
{
namespace gui
{
Button::Button(bool isSwitchButton) :
	AbstractButton(isSwitchButton)
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
		textOffset = skin->GetSunkenOffset();

	auto state = GetState();
	PaintOptions options;
	options.palette = palette;
	skin->DrawControl(r, this, GetFinalRect(), EGUIControl::Button, state, options);

	FontRenderSettings settings;
	settings.color = palette.GetWindowText(GetState());
	m_Text.Render(
		r, GetFont(),
		settings, false, false, GetAlignment(),
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

core::Name Button::GetReferableType() const
{
	static const core::Name name("lux.gui.Button");
	return name;
}

} // namespace gui
} // namespace lux