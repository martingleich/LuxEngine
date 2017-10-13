#include "gui/elements/GUIStaticText.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.gui.StaticText", lux::gui::StaticText);

namespace lux
{
namespace gui
{
StaticText::StaticText() :
	m_WordWrap(false),
	m_DrawBackground(false),
	m_OverwriteColor(true),
	m_ClipTextInside(false)
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

/*
void StaticText::SetDrawBackground(bool draw)
{
	m_DrawBackground = draw;
}

bool StaticText::GetDrawBackground() const
{
	return m_DrawBackground;
}

void StaticText::SetWordWrap(bool wrap)
{
	m_WordWrap = wrap;
}

bool StaticText::GetWordWrap() const
{
	return m_WordWrap;
}

void StaticText::SetClipTextInside(bool clip)
{
	m_ClipTextInside = clip;
}

bool StaticText::GetClipTextInside() const
{
	return m_ClipTextInside;
}

void StaticText::SetFitSizeToText(bool fit)
{
	m_FitSizeToText = fit;
}

bool StaticText::GetFitSizeToText() const
{
	return m_FitSizeToText;
}
*/

} // namespace gui
} // namespace lux