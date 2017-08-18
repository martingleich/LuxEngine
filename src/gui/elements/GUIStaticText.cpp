#include "gui/elements/GUIStaticText.h"
#include "gui/GUISkin.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.gui.StaticText", lux::gui::StaticText);

namespace lux
{
namespace gui
{
StaticText::StaticText() :
	m_Align(Font::EAlign::TopLeft),
	m_Color(0, 0, 0),
	m_Background(255, 255, 255),
	m_RebreakText(true),
	m_WordWrap(false),
	m_DrawBackground(false),
	m_OverwriteColor(true),
	m_ClipTextInside(false)
{
}

StaticText::~StaticText()
{
}

void StaticText::Paint(Renderer* r)
{
	if(m_Text.IsEmpty())
		return;
	auto font = GetActiveFont();
	if(!font)
		return;

	auto skin = GetSkin();
	if(m_DrawBackground)
		skin->DrawPrimitive(r, GetState(), EGUIPrimitive::StaticText, GetFinalRect(), m_Background);

	EnsureBrokenText();

	float lineHeight = font->GetLineDistance()*font->GetScaling()*font->GetFontHeight();
	float totalHeight = lineHeight * m_BrokenText.Size();

	auto area = GetFinalInnerRect();
	math::Vector2F cursor;
	if(TestFlag(m_Align, Font::EAlign::VTop))
		cursor.y = area.top;
	else if(TestFlag(m_Align, Font::EAlign::VCenter))
		cursor.y = (area.top + area.bottom) / 2 - totalHeight / 2;
	else if(TestFlag(m_Align, Font::EAlign::VBottom))
		cursor.y = area.bottom - totalHeight;

	if(TestFlag(m_Align, Font::EAlign::HLeft))
		cursor.x = area.left;
	else if(TestFlag(m_Align, Font::EAlign::HCenter))
		cursor.x = (area.left + area.right) / 2;
	else if(TestFlag(m_Align, Font::EAlign::HRight))
		cursor.x = area.right;

	// Each drawn line is align to the given horizontal alignment and vTop
	Font::EAlign lineAlign = Font::EAlign::VTop | (m_Align & ~(Font::EAlign::VCenter | Font::EAlign::VBottom | Font::EAlign::VTop));
	auto clip = m_ClipTextInside ? &m_InnerRect : nullptr;

	video::Colorf color;
	if(!m_OverwriteColor) {
		if(IsEnabled())
			color = skin->textColor;
		else
			color *= skin->disabledTextColor;
	} else {
		color = m_Color;
	}

	for(auto& line : m_BrokenText) {
		if(!clip || cursor.y + lineHeight >= clip->top)
			r->DrawText(font, line, cursor, lineAlign, color.ToHex(), clip);
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

void StaticText::SetOverwriteFont(Font* f)
{
	m_OverwriteFont = f;
	m_RebreakText = true;
}

StrongRef<Font> StaticText::GetOverwriteFont() const
{
	return m_OverwriteFont;
}

StrongRef<Font> StaticText::GetActiveFont() const
{
	if(m_OverwriteFont)
		return m_OverwriteFont;
	else
		return GetSkin()->defaultFont;
}

void StaticText::SetColor(video::Color color)
{
	m_Color = color;
}

video::Color StaticText::GetColor() const
{
	return m_Color;
}

void StaticText::SetAlignment(Font::EAlign align)
{
	m_Align = align;
}

Font::EAlign StaticText::GetAlignment() const
{
	return m_Align;
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

void StaticText::EnsureBrokenText() const
{
	auto font = GetActiveFont();
	if((Font*)font != (Font*)m_LastBrokenFont)
		m_RebreakText = true;
	m_LastBrokenFont = font.GetWeak();
	if(!m_RebreakText)
		return;
	m_RebreakText = false;
	m_BrokenText.Clear();

	if(m_Text.IsEmpty())
		return;

	auto width = GetFinalInnerWidth();
	auto& brokenText = m_BrokenText;
	core::Array<float> carets;
	auto AddBrokenLine = [&](String&& line) {
		if(!m_WordWrap || font->GetTextWidth(line) <= width) {
			m_BrokenText.PushBack(std::move(line));
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
					offset += carets[id];
					lineFirst = prevBreakPoint + 1;
				}
			}
			if(lineFirst != line.End())
				brokenText.PushBack(std::move(String(lineFirst, line.End())));
		}

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
}

void StaticText::OnInnerRectChange()
{
	Element::OnInnerRectChange();
	m_RebreakText = true;
}

} // namespace gui
} // namespace lux