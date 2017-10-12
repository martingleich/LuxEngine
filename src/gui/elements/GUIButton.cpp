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

	r->DrawText(GetFont(),
		m_Text,
		GetFinalInnerRect() + textOffset,
		GetAlignment(),
		palette.GetWindowText(state));
}

core::Name Button::GetReferableType() const
{
	static const core::Name name = "lux.gui.Button";
	return name;
}

} // namespace gui
} // namespace lux