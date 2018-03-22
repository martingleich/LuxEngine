#ifndef INCLUDED_GUI_GUI_TEXT_CONTAINER_H
#define INCLUDED_GUI_GUI_TEXT_CONTAINER_H
#include "gui/Font.h"
#include "gui/GUIAlign.h"

namespace lux
{
namespace gui
{
class Renderer;

//! Text in a box
/**
Helper class for gui implementations. Makes it easy to render text with automatic
clipping, wordwrapping, diffrent alignments and lazy updates.
*/
class TextContainer
{
public:
	LUX_API TextContainer();
	LUX_API ~TextContainer();

	//! Rebreak the text into lines.
	/**
	Must only be called when the text is changed via the Text() method,
	in all other cases called automatically.
	\param firstLine All lines before this one didn't change.
	*/
	LUX_API void Rebreak(int firstLine = 0);

	//! Update all data for the text
	/**
	Updates lines, line-widths, total text box.
	And set font for following render calls.
	Should be called always before rendering.
	\param font The font used to render the text.
	\param settings The font render settings.
	\param wordWrap Should automatic word wrapping be performed.
	\param textBoxSize The size of the the textbox, only used if wordWrap is true
	*/
	LUX_API void Ensure(
		Font* font,
		const FontRenderSettings& settings,
		bool wordWrap,
		const math::Dimension2F& textBoxSize);

	//! Renders text after a call to Ensure
	/**
	Ensure must be called before this method.
	Uses FontRenderSettings and Font set via Ensure.
	\param r Renderer used to draw the text
	\param align Alignment of the text inside the textbox
	\param textBox The textbox to draw the text in
	\param clipBox Additional box to clip the text against
	*/
	LUX_API void Render(
		gui::Renderer* r,
		EAlign align,
		const math::RectF& textBox,
		const math::RectF* clipBox=nullptr);

	//! Renders and Ensures text in one call
	/**
	\param r Renderer used to draw the text
	\param font The font used to render the text.
	\param settings The font render settings.
	\param wordWrap Should automatic word wrapping be performed.
	\param align Alignment of the text inside the textbox
	\param textBox The textbox to draw the text in
	\param clipBox Additional box to clip the text against
	*/
	LUX_API void Render(
		gui::Renderer* r,
		Font* font,
		const FontRenderSettings& settings,
		bool wordWrap,
		EAlign align,
		const math::RectF& textBox,
		const math::RectF* clipBox=nullptr);

	LUX_API int GetLineCount() const;
	LUX_API core::Range<core::String::ConstIterator> GetLine(int i) const;

	LUX_API float GetLineWidth(int i) const;
	LUX_API math::Dimension2F GetDimension() const;

	LUX_API void SetText(const core::String& str);
	LUX_API const core::String& GetText() const;

	//! Allow mutable access to the text, without copieing.
	/**
	Rebreak should be called after changing the text.
	*/
	LUX_API core::String& Text();

private:
	struct Line
	{
		Line() {}
		Line(core::Range<core::String::ConstIterator> l, float w) :
			line(l),
			width(w)
		{
		}
		core::Range<core::String::ConstIterator> line;
		float width;
	};
	core::String m_Text;
	int m_Rebreak;

	// Cached data
	WeakRef<Font> m_Font;
	bool m_Wrap;
	math::Dimension2F m_TextBoxSize;
	gui::FontRenderSettings m_FontSettings;

	// Generated data
	core::Array<Line> m_BrokenText;
	math::Dimension2F m_TextDim;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_GUI_TEXT_CONTAINER_H
