#include "gui/elements/GUIStaticText.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.gui.StaticText", lux::gui::StaticText);

namespace lux
{
namespace gui
{
StaticText::StaticText() :
	m_DrawBackground(false),
	m_OverwriteColor(true),
	m_ClipTextInside(false),
	m_FitSizeToText(false),
	m_WordWrap(true)
{
	SetAlignment(EAlign::TopLeft);
}

StaticText::~StaticText()
{
}

void StaticText::Paint(Renderer* r)
{
	if(m_Text.IsEmpty())
		return;
	auto font = GetFont();
	if(!font)
		return;

	auto skin = GetSkin();
	auto palette = GetFinalPalette();
	auto state = GetState();

	FontRenderSettings settings;
	settings.color = palette.GetWindowText(state);
	m_TextContainer.Ensure(font, settings, m_FitSizeToText ? false : m_WordWrap, GetFinalInnerRect(), GetText());

	if(m_FitSizeToText) {
		math::Dimension2F size = m_TextContainer.GetDimension();
		SetInnerSize(PixelDimension(size.width, size.height));
	}

	PaintOptions po;
	po.palette = palette;
	if(m_DrawBackground)
		skin->DrawControl(r, this, GetFinalRect(), EGUIControl::StaticText, state, po);

	m_TextContainer.Render(r, GetAlignment(), m_ClipTextInside, GetFinalInnerRect());
}

void StaticText::FitSizeToText()
{
	FontRenderSettings settings;
	m_TextContainer.Ensure(GetFont(), settings, m_WordWrap, GetFinalInnerRect(), GetText());
	math::Dimension2F size = m_TextContainer.GetDimension();
	SetInnerSize(PixelDimension(size.width, size.height));
}

void StaticText::SetText(const String& text)
{
	Element::SetText(text);
	m_TextContainer.Rebreak();
}

core::Name StaticText::GetReferableType() const
{
	static const core::Name name = "lug.gui.StaticText";
	return name;
}

} // namespace gui
} // namespace lux