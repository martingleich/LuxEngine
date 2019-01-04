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

void TextContainer::Rebreak(int firstLine)
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
	int rebreakText = m_Rebreak;
	if(!m_BrokenText.IsEmpty() && m_Text.Data() != m_BrokenText[0].line.Data()) {
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

	if(rebreakText == std::numeric_limits<int>::max())
		return;

	m_Rebreak = std::numeric_limits<int>::max();

	auto width = m_TextBoxSize.width;
	auto& textDim = m_TextDim;
	textDim = math::Dimension2F(0, 0);
	core::Array<float> carets;
	bool keepBreaking = true;
	int lineId = rebreakText;

	auto AddBrokenLine = [&](const core::StringView& str, float width) {
		if(m_BrokenText.Size() > lineId) {
			m_BrokenText[lineId] = Line(str, width);
		} else {
			m_BrokenText.EmplaceBack(str, width);
		}
		++lineId;
		textDim.width = math::Max(textDim.width, width);
	};

	auto AddLine = [&](const core::StringView& line) {
		if(line.IsEmpty())
			return;
		auto lineWidth = font->GetTextWidth(settings, line);
		if(!wordWrap || lineWidth <= width) {
			AddBrokenLine(line, lineWidth);
		} else {
			auto prevBreakPoint = line.CodePoints().End();
			carets.Clear();
			m_Font->GetTextCarets(settings, line, carets);
			float offset = 0.0f;
			auto lineFirst = line.CodePoints().First();
			int id = 0;
			for(auto jt = line.CodePoints().First(); jt != line.CodePoints().End(); ++jt) {
				++id;
				if(*jt == ' ') {
					prevBreakPoint = jt;
				} else if(carets[id] - offset > width && prevBreakPoint != line.CodePoints().End()) {
					auto lineBegin = lineFirst.Pointer();
					auto lineEnd = prevBreakPoint.Pointer();
					AddBrokenLine(core::StringView(lineBegin, lineEnd-lineBegin), carets[id] - offset);
					offset = carets[id];
					lineFirst = prevBreakPoint + 1;
					prevBreakPoint = line.CodePoints().End();
				}
			}
			if(lineFirst != line.CodePoints().End()) {
				auto lineBegin = lineFirst.Pointer();
				auto lineEnd = line.Bytes().End();
				AddBrokenLine(core::StringView(lineBegin, lineEnd-lineBegin), carets[id] - offset);
			}
		}

		float lineHeight = settings.lineDistance*settings.scale*font->GetFontHeight();
		textDim.height = lineHeight * m_BrokenText.Size();
	};

	auto textFirst = m_BrokenText.IsEmpty() ? m_Text.CodePoints().First() : m_BrokenText[rebreakText].line.CodePoints().First();
	auto first = m_Text.CodePoints().First();
	auto end = first;
	core::String line;
	for(auto it = textFirst; it != m_Text.CodePoints().End() && keepBreaking;) {
		if(*it == '\n') {
			AddLine(core::StringView(first.Pointer(), end.Pointer()-first.Pointer()));
			++it;
			end = first = it;
		} else {
			++it;
			end = it;
		}
	}

	if(keepBreaking)
		AddLine(core::StringView(first.Pointer(), end.Pointer()-first.Pointer()));

	if(keepBreaking)
		m_BrokenText.Resize(lineId);
}

void TextContainer::Render(
	gui::Renderer* r,
	EAlign align,
	const math::RectF& textBox,
	const math::RectF* clipBox)
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

	for(auto line : m_BrokenText) {
		auto lineRange = line.line;
		auto lineWidth = line.width;
		if(TestFlag(align, EAlign::HLeft))
			cursor.x = textBox.left;
		else if(TestFlag(align, EAlign::HCenter))
			cursor.x = (textBox.left + textBox.right) / 2 - 0.5f*lineWidth;
		else if(TestFlag(align, EAlign::HRight))
			cursor.x = textBox.right - lineWidth;

		if(!clipBox || cursor.y + lineHeight >= clipBox->top)
			r->DrawText(m_Font, m_FontSettings, lineRange, cursor, clipBox);
		cursor.y += lineHeight;
		if(clipBox && cursor.y > clipBox->bottom)
			break;
	}
}

void TextContainer::Render(
	gui::Renderer* r,
	Font* font,
	const FontRenderSettings& settings,
	bool wordWrap,
	EAlign align,
	const math::RectF& textBox,
	const math::RectF* clipBox)
{
	Ensure(font, settings, wordWrap, textBox.GetSize());
	Render(r, align, textBox, clipBox);
}

int TextContainer::GetLineCount() const
{
	return m_BrokenText.Size();
}

core::StringView TextContainer::GetLine(int i) const
{
	return m_BrokenText[i].line;
}

float TextContainer::GetLineWidth(int i) const
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