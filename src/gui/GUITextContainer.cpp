#include "gui/GUITextContainer.h"
#include "gui/GUIRenderer.h"

namespace lux
{
namespace gui
{

TextContainer::TextContainer()
{
	m_Rebreak = 0;
}

TextContainer::~TextContainer()
{
}

void TextContainer::Rebreak(size_t firstLine)
{
	m_Rebreak = firstLine;
}

void TextContainer::Ensure(
	Font* font,
	const FontRenderSettings& settings,
	bool wordWrap,
	const math::Dimension2F& textBoxSize)
{
	if(!font || m_Text.IsEmpty()) {
		m_BrokenText.Clear();
		return;
	}
	size_t rebreakText = m_Rebreak;
	if(!m_BrokenText.IsEmpty() && m_Text.First() != m_BrokenText[0].line.First()) {
		m_BrokenText.Clear();
		rebreakText = 0;
	}

	if(font != m_Font) {
		rebreakText = 0;
		m_Font = font;
	}
	if(wordWrap != m_Wrap) {
		rebreakText = 0;
		m_Wrap = wordWrap;
	}
	if(textBoxSize != m_TextBoxSize) {
		rebreakText = 0;
		m_TextBoxSize = textBoxSize;
	}
	if(m_FontSettings.charDistance != settings.charDistance ||
		m_FontSettings.lineDistance != settings.lineDistance ||
		m_FontSettings.scale != settings.scale ||
		m_FontSettings.wordDistance != settings.wordDistance) {
		rebreakText = 0;
	}
	m_FontSettings = settings; // Copy outside of it, to get non-geometric member(for example color)

	if(rebreakText == std::numeric_limits<size_t>::max())
		return;

	m_Rebreak = std::numeric_limits<size_t>::max();

	auto width = m_TextBoxSize.width;
	auto& textDim = m_TextDim;
	textDim = math::Dimension2F(0, 0);
	core::Array<float> carets;
	bool keepBreaking = true;
	size_t lineId = rebreakText;
	auto AddBrokenLine = [&](core::Range<core::String::ConstIterator> str, float width) {
		if(m_BrokenText.Size() > lineId) {
			if(m_BrokenText[lineId].line == str && m_BrokenText[lineId].width == width)
				keepBreaking = false;
			m_BrokenText[lineId] = Line(str, width);
		} else {
			m_BrokenText.EmplaceBack(str, width);
		}
		++lineId;
		textDim.width = math::Max(textDim.width, width);
	};
	auto AddLine = [&](core::Range<core::String::ConstIterator> line) {
		if(line.begin() == line.end())
			return;
		auto lineWidth = font->GetTextWidth(settings, line);
		if(!wordWrap || lineWidth <= width) {
			AddBrokenLine(line, lineWidth);
		} else {
			core::String::ConstIterator prevBreakPoint = line.End();
			carets.Clear();
			m_Font->GetTextCarets(settings, line, carets);
			float offset = 0.0f;
			core::String::ConstIterator lineFirst = line.First();
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

	auto textFirst = m_BrokenText.IsEmpty() ? m_Text.First() : m_BrokenText[rebreakText].line.begin();
	auto first = m_Text.First();
	auto end = first;
	core::String line;
	for(auto it = textFirst; it != m_Text.End() && keepBreaking;) {
		if(*it == '\n') {
			AddLine(core::MakeRange(first, end));
			++it;
			end = first = it;
		} else {
			++it;
			end = it;
		}
	}

	if(keepBreaking)
		AddLine(core::MakeRange(first, end));

	if(keepBreaking)
		m_BrokenText.Resize(lineId);
}

void TextContainer::Render(
	gui::Renderer* r,
	EAlign align,
	bool clipTextInside,
	const math::RectF& textBox)
{
	if(m_Text.IsEmpty())
		return;
	float lineHeight = m_FontSettings.lineDistance*m_FontSettings.scale*m_Font->GetFontHeight();
	float totalHeight = lineHeight * m_BrokenText.Size();

	math::Vector2F cursor;
	if(TestFlag(align, EAlign::VTop))
		cursor.y = textBox.top;
	else if(TestFlag(align, EAlign::VCenter))
		cursor.y = (textBox.top + textBox.bottom) / 2 - totalHeight / 2;
	else if(TestFlag(align, EAlign::VBottom))
		cursor.y = textBox.bottom - totalHeight;

	auto clip = clipTextInside ? &textBox : nullptr;
	for(auto line : m_BrokenText) {
		auto lineRange = line.line;
		auto lineWidth = line.width;
		if(TestFlag(align, EAlign::HLeft))
			cursor.x = textBox.left;
		else if(TestFlag(align, EAlign::HCenter))
			cursor.x = (textBox.left + textBox.right) / 2 - 0.5f*lineWidth;
		else if(TestFlag(align, EAlign::HRight))
			cursor.x = textBox.right - lineWidth;

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
	const math::RectF& textBox)
{
	Ensure(font, settings, wordWrap, textBox.GetSize());
	Render(r, align, clipTextInside, textBox);
}

size_t TextContainer::GetLineCount() const
{
	return m_BrokenText.Size();
}

core::Range<core::String::ConstIterator> TextContainer::GetLine(size_t i) const
{
	return m_BrokenText[i].line;
}

float TextContainer::GetLineWidth(size_t i) const
{
	return m_BrokenText[i].width;
}

math::Dimension2F TextContainer::GetDimension() const
{
	return m_TextDim;
}

void TextContainer::SetText(const core::String& str)
{
	m_Text = str;
	Rebreak();
}
const core::String& TextContainer::GetText() const
{
	return m_Text;
}

core::String& TextContainer::Text()
{
	return m_Text;
}

} // namespace gui
} // namespace lux 