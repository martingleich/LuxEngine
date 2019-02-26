#include "gui/GUITextContainer.h"
#include "gui/GUIRenderer.h"

namespace lux
{
namespace gui
{

TextContainer::TextContainer()
{
	m_Update = true;
}

TextContainer::~TextContainer()
{
}

void TextContainer::Rebreak()
{
	m_Update = true;
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

	bool update = m_Update;
	if(font != m_Font) {
		update = true;
		m_Font = font;
	}
	if(wordWrap != m_Wrap) {
		update = true;
		m_Wrap = wordWrap;
	}
	if(textBoxSize != m_TextBoxSize) {
		update = true;
		m_TextBoxSize = textBoxSize;
	}
	if(m_FontSettings.charDistance != settings.charDistance ||
		m_FontSettings.lineDistance != settings.lineDistance ||
		m_FontSettings.scale != settings.scale ||
		m_FontSettings.wordDistance != settings.wordDistance) {
		update = true;
	}
	m_FontSettings = settings; // Copy outside of it, to get non-geometric member(for example color)

	if(!update)
		return;
	m_BrokenText.Clear();
	m_Update = false;

	float textBoxWidth = m_TextBoxSize.width;
	float maxWidth = 0;
	core::Array<FontCaret> carets;

	auto AddBrokenLine = [&](core::StringView line, float width) {
		m_BrokenText.EmplaceBack(line, width);
		maxWidth = math::Max(maxWidth, width);
	};

	auto AddWordWrapedLine = [&](core::StringView line) {
		carets.Clear();
		m_Font->GetTextCarets(settings, line, carets);

		int lastBreakpoint = -1; // Last breakpoint in bytes
		float lastLineDistance = 0.0f; // Distance from the start of line to the end of the last line.
		int lineBegin = 0;
		for(auto& car : core::SliceRange(carets, 0, -2)) {
			if(line[car.offset] == ' ') { // Possible break point. (Access as byte is possible since <space> is a ascii character)
				lastBreakpoint = car.offset;
			} else if(car.distance - lastLineDistance > textBoxWidth && lastBreakpoint != -1) {
				// If the line overflows and a breakpoint is availble.
				// Break the line at the lastBreakpoint.
				AddBrokenLine(line.SubString(lineBegin, lastBreakpoint - lineBegin), car.distance - lastLineDistance);
				lastLineDistance = car.distance;
				lineBegin = lastBreakpoint + 1; // Add 1 for the 1 Byte size of <space>
				lastBreakpoint = -1;
			}
		}

		if(lineBegin != line.Size())
			AddBrokenLine(line.SubString(lineBegin, line.Size() - lineBegin), carets.Back().distance - lastLineDistance);
	};

	m_Text.AsView().BasicSplit("\n", core::EStringSplit::Normal, [&](core::StringView line) {
		auto lineWidth = font->GetTextWidth(settings, line);
		if(!wordWrap || lineWidth <= textBoxWidth)
			AddBrokenLine(line, lineWidth);
		else
			AddWordWrapedLine(line);
		return true;
	});

	float lineHeight = settings.lineDistance*settings.scale*font->GetFontHeight();
	m_TextDim.height = lineHeight * m_BrokenText.Size();
	m_TextDim.width = maxWidth;
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
		auto lineWidth = line.width;
		if(TestFlag(align, EAlign::HLeft))
			cursor.x = textBox.left;
		else if(TestFlag(align, EAlign::HCenter))
			cursor.x = (textBox.left + textBox.right) / 2 - 0.5f*lineWidth;
		else if(TestFlag(align, EAlign::HRight))
			cursor.x = textBox.right - lineWidth;

		if(!clipBox || cursor.y + lineHeight >= clipBox->top)
			r->DrawText(m_Font, m_FontSettings, line.text, cursor, clipBox);
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
	lxAssert(!m_Update);
	return m_BrokenText[i].text;
}

float TextContainer::GetLineWidth(int i) const
{
	lxAssert(!m_Update);
	return m_BrokenText[i].width;
}

math::Dimension2F TextContainer::GetDimension() const
{
	lxAssert(!m_Update);
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