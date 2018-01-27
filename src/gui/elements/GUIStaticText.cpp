#include "gui/elements/GUIStaticText.h"
#include "gui/GUISkin.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::StaticText, "lux.gui.StaticText");

namespace lux
{
namespace gui
{
StaticText::StaticText() :
	m_DrawBackground(false),
	m_OverwriteColor(true),
	m_ClipTextInside(false),
	m_FitSizeToText(true),
	m_WordWrap(false)
{
	SetAlignment(EAlign::TopLeft);
	SetFocusable(false);
}

StaticText::~StaticText()
{
}

void StaticText::Paint(Renderer* r)
{
	if(GetText().IsEmpty())
		return;
	auto font = GetFont();
	if(!font)
		return;

	auto skin = GetSkin();
	auto palette = GetFinalPalette();
	auto state = GetState();

	FontRenderSettings settings;
	settings.color = palette.GetWindowText(state);
	m_TextContainer.Ensure(font, settings, m_FitSizeToText ? false : m_WordWrap, GetFinalInnerRect().GetSize());

	if(m_FitSizeToText) {
		math::Dimension2F size = m_TextContainer.GetDimension();
		SetInnerSize(PixelDimension(size.width, size.height));
	}

	PaintOptions po;
	po.palette = palette;
	if(m_DrawBackground)
		skin->DrawControl(r, this, GetFinalRect(), EGUIControl::StaticText, state, po);

	auto rect = GetFinalRect();
	m_TextContainer.Render(r, GetAlignment(), rect, m_ClipTextInside ? &rect : nullptr);
}

void StaticText::FitSizeToText()
{
	FontRenderSettings settings;
	m_TextContainer.Ensure(GetFont(), settings, m_WordWrap, GetFinalInnerRect().GetSize());
	math::Dimension2F size = m_TextContainer.GetDimension();
	SetInnerSize(PixelDimension(size.width, size.height));
}

void StaticText::SetText(const core::String& text)
{
	m_TextContainer.SetText(text);
}

const core::String& StaticText::GetText() const
{
	return m_TextContainer.GetText();
}

} // namespace gui
} // namespace lux