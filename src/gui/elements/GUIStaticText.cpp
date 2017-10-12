#include "gui/elements/GUIStaticText.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.gui.StaticText", lux::gui::StaticText);

namespace lux
{
namespace gui
{
StaticText::StaticText() :
	m_RebreakText(true),
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
	auto align = GetAlignment();
	PaintOptions po;
	po.palette = palette;
	if(m_DrawBackground)
		skin->DrawControl(r, this, GetFinalRect(), EGUIControl::StaticText, state, po);

	EnsureBrokenText();

	float lineHeight = font->GetLineDistance()*font->GetScaling()*font->GetFontHeight();
	float totalHeight = lineHeight * m_BrokenText.Size();

	auto area = GetFinalInnerRect();
	math::Vector2F cursor;
	if(TestFlag(align, EAlign::VTop))
		cursor.y = area.top;
	else if(TestFlag(align, EAlign::VCenter))
		cursor.y = (area.top + area.bottom) / 2 - totalHeight / 2;
	else if(TestFlag(align, EAlign::VBottom))
		cursor.y = area.bottom - totalHeight;

	if(TestFlag(align, EAlign::HLeft))
		cursor.x = area.left;
	else if(TestFlag(align, EAlign::HCenter))
		cursor.x = (area.left + area.right) / 2;
	else if(TestFlag(align, EAlign::HRight))
		cursor.x = area.right;

	// Each drawn line is align to the given horizontal alignment and vTop
	EAlign lineAlign = EAlign::VTop | (align & ~(EAlign::VCenter | EAlign::VBottom | EAlign::VTop));
	auto clip = m_ClipTextInside ? &m_InnerRect : nullptr;

	video::Color color = palette.GetWindowText(state);
	for(auto& line : m_BrokenText) {
		if(!clip || cursor.y + lineHeight >= clip->top)
			r->DrawText(font, line, cursor, lineAlign, color, clip);
		cursor.y += lineHeight;
		if(clip && cursor.y > clip->bottom)
			break;
	}
}

core::Name StaticText::GetReferableType() const
{
	static const core::Name name = "lug.gui.StaticText";
	return name;
}

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
	m_RebreakText = true;
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

void StaticText::SetText(const String& text)
{
	Element::SetText(text);
	m_RebreakText = true;
}

void StaticText::EnsureBrokenText()
{
	auto font = GetFont();
	if((Font*)font != (Font*)m_LastBrokenFont)
		m_RebreakText = true;
	m_LastBrokenFont = font;
	if(!m_RebreakText)
		return;
	m_RebreakText = false;
	m_BrokenText.Clear();

	if(m_Text.IsEmpty())
		return;

	auto width = GetFinalInnerWidth();
	auto& brokenText = m_BrokenText;
	auto& textWidth = m_TextWidth;
	auto& textHeight = m_TextHeight;
	bool wordWrap = m_FitSizeToText ? false : m_WordWrap;
	textWidth = 0;
	textHeight = 0;
	core::Array<float> carets;
	auto AddBrokenLine = [&](String&& line) {
		auto lineWidth = font->GetTextWidth(line);
		if(!wordWrap || lineWidth <= width) {
			m_BrokenText.PushBack(std::move(line));
			textWidth = math::Max(textWidth, lineWidth);
		} else {
			String::ConstIterator prevBreakPoint = line.End();
			carets.Clear();
			font->GetTextCarets(line, carets);
			float offset = 0.0f;
			String::ConstIterator lineFirst = line.First();
			size_t id = 0;
			for(auto jt = line.First(); jt != line.End(); ++jt) {
				++id;
				if(*jt == ' ') {
					prevBreakPoint = jt;
				} else if(carets[id] - offset > width && prevBreakPoint != line.End()) {
					brokenText.PushBack(std::move(String(lineFirst, prevBreakPoint)));
					textWidth = math::Max(textWidth, carets[id] - offset);
					offset += carets[id];
					lineFirst = prevBreakPoint + 1;
				}
			}
			if(lineFirst != line.End())
				brokenText.PushBack(std::move(String(lineFirst, line.End())));
		}

		float lineHeight = font->GetLineDistance()*font->GetScaling()*font->GetFontHeight();
		textHeight = lineHeight * m_BrokenText.Size();

		line.Clear();
	};

	auto& str = GetText();
	String::ConstIterator first = str.First();
	String::ConstIterator end = first;
	String line;
	for(auto it = str.First(); it != str.End();) {
		if(*it == '\n') {
			line.Append(first, end);
			AddBrokenLine(std::move(line));
			++it;
			end = first = it;
		} else {
			++it;
			end = it;
		}
	}

	line.Append(first, end);
	AddBrokenLine(std::move(line));

	if(m_FitSizeToText) {
		math::Dimension2F size;
		size.width = m_TextWidth;
		size.height = m_TextHeight;

		SetInnerSize(PixelDimension(size.width, size.height));
	}
}

void StaticText::OnInnerRectChange()
{
	Element::OnInnerRectChange();
	m_RebreakText = true;
}

void StaticText::SetFitSizeToText(bool fit)
{
	m_FitSizeToText = fit;
}

bool StaticText::GetFitSizeToText() const
{
	return m_FitSizeToText;
}

void StaticText::FitSizeToText()
{
	m_WordWrap = false;
	EnsureBrokenText();
	math::Dimension2F size;
	size.width = m_TextWidth;
	size.height = m_TextHeight;

	SetInnerSize(PixelDimension(size.width, size.height));
}

} // namespace gui
} // namespace lux