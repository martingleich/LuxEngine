#include "gui/GUITextContainer.h"
#include "gui/GUIRenderer.h"

namespace lux
{
namespace gui
{

TextContainer::TextContainer()
{
	m_Rebreak = true;
}

TextContainer::~TextContainer()
{
}

void TextContainer::Rebreak()
{
	m_Rebreak = true;
}

void TextContainer::Ensure(
	Font* font,
	const FontRenderSettings& settings,
	bool wordWrap,
	const math::RectF& rect,
	const String& text)
{
	if(!font || text.IsEmpty())
		return;
	bool rebreakText = m_Rebreak;
	if(font != m_Font) {
		rebreakText = true;
		m_Font = font;
	}
	if(wordWrap != m_Wrap) {
		rebreakText = true;
		m_Wrap = wordWrap;
	}
	if(rect.GetSize() != m_TextBoxSize) {
		rebreakText = true;
		m_TextBoxSize = rect.GetSize();
	}
	if(m_FontSettings.charDistance != settings.charDistance ||
		m_FontSettings.lineDistance != settings.lineDistance ||
		m_FontSettings.scale != settings.scale ||
		m_FontSettings.wordDistance != settings.wordDistance) {
		rebreakText = true;
	}
	m_FontSettings = settings; // Copy outside of it, to get non-geometric member(for example color)

	if(!rebreakText)
		return;

	m_Rebreak = false;
	m_BrokenText.Clear();

	auto width = m_TextBoxSize.width;
	auto& textDim = m_TextDim;
	textDim = math::Dimension2F(0, 0);
	core::Array<float> carets;
	auto AddBrokenLine = [&](core::Range<String::ConstIterator> str, float width) {
		m_BrokenText.PushBack(str);
		m_LineSizes.PushBack(width);
		textDim.width = math::Max(textDim.width, width);
	};
	auto AddLine = [&](core::Range<String::ConstIterator> line) {
		if(line.begin() == line.end())
			return;
		auto lineWidth = font->GetTextWidth(settings, line);
		if(!wordWrap || lineWidth <= width) {
			AddBrokenLine(line, lineWidth);
		} else {
			String::ConstIterator prevBreakPoint = line.End();
			carets.Clear();
			m_Font->GetTextCarets(settings, line, carets);
			float offset = 0.0f;
			String::ConstIterator lineFirst = line.First();
			size_t id = 0;
			for(auto jt = line.First(); jt != line.End(); ++jt) {
				++id;
				if(*jt == ' ') {
					prevBreakPoint = jt;
				} else if(carets[id] - offset > width && prevBreakPoint != line.End()) {
					AddBrokenLine(core::MakeRange(lineFirst, prevBreakPoint), carets[id] - offset);
					offset = carets[id];
					lineFirst = prevBreakPoint + 1;
					prevBreakPoint = line.End();
				}
			}
			if(lineFirst != line.End())
				AddBrokenLine(core::MakeRange(lineFirst, line.End()), carets[id] - offset);
		}

		float lineHeight = settings.lineDistance*settings.scale*font->GetFontHeight();
		textDim.height = lineHeight * m_BrokenText.Size();
	};

	String::ConstIterator first = text.First();
	String::ConstIterator end = first;
	String line;
	for(auto it = text.First(); it != text.End();) {
		if(*it == '\n') {
			AddLine(core::MakeRange(first, end));
			++it;
			end = first = it;
		} else {
			++it;
			end = it;
		}
	}

	AddLine(core::MakeRange(first, end));
}

void TextContainer::Render(
	gui::Renderer* r,
	EAlign align,
	bool clipTextInside,
	const math::RectF& rect)
{
	float lineHeight = m_FontSettings.lineDistance*m_FontSettings.scale*m_Font->GetFontHeight();
	float totalHeight = lineHeight * m_BrokenText.Size();

	math::Vector2F cursor;
	if(TestFlag(align, EAlign::VTop))
		cursor.y = rect.top;
	else if(TestFlag(align, EAlign::VCenter))
		cursor.y = (rect.top + rect.bottom) / 2 - totalHeight / 2;
	else if(TestFlag(align, EAlign::VBottom))
		cursor.y = rect.bottom - totalHeight;

	auto clip = clipTextInside ? &rect : nullptr;
	for(auto line : core::ZipRange(m_BrokenText, m_LineSizes)) {
		auto lineRange = line.get<0>();
		auto lineWidth = line.get<1>();
		if(TestFlag(align, EAlign::HLeft))
			cursor.x = rect.left;
		else if(TestFlag(align, EAlign::HCenter))
			cursor.x = (rect.left + rect.right) / 2 - 0.5f*lineWidth;
		else if(TestFlag(align, EAlign::HRight))
			cursor.x = rect.right - lineWidth;

		if(!clip || cursor.y + lineHeight >= clip->top)
			r->DrawText(m_Font, m_FontSettings, lineRange, cursor, clip);
		cursor.y += lineHeight;
		if(clip && cursor.y > clip->bottom)
			break;
	}
}

void TextContainer::Render(
	gui::Renderer* r,
	Font* font,
	const FontRenderSettings& settings,
	bool wordWrap,
	bool clipTextInside,
	EAlign align,
	const math::RectF& rect,
	const String& text)
{
	Ensure(font, settings, wordWrap, rect, text);
	Render(r, align, clipTextInside, rect);
}

size_t TextContainer::GetLineCount() const
{
	return m_BrokenText.Size();
}

core::Range<String::ConstIterator> TextContainer::GetLine(size_t i) const
{
	return m_BrokenText[i];
}

float TextContainer::GetLineWidth(size_t i) const
{
	return m_LineSizes[i];
}

math::Dimension2F TextContainer::GetDimension() const
{
	return m_TextDim;
}

} // namespace gui
} // namespace lux 