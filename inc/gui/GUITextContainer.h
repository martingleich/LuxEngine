#ifndef INCLUDED_GUI_GUI_TEXT_CONTAINER_H
#define INCLUDED_GUI_GUI_TEXT_CONTAINER_H
#include "gui/Font.h"

namespace lux
{
namespace gui
{
class Renderer;

class TextContainer
{
public:
	LUX_API TextContainer();
	LUX_API ~TextContainer();

	LUX_API void Rebreak();

	LUX_API void Ensure(
		Font* font,
		const FontRenderSettings& settings,
		bool wordWrap,
		const math::RectF& rect,
		const String& text);

	LUX_API void Render(
		gui::Renderer* r,
		EAlign align,
		bool clipTextInside,
		const math::RectF& rect);

	LUX_API void Render(
		gui::Renderer* r,
		Font* font,
		const FontRenderSettings& settings,
		bool wordWrap,
		bool clipTextInside,
		EAlign align,
		const math::RectF& rect,
		const String& text);

	LUX_API size_t GetLineCount() const;
	LUX_API core::Range<String::ConstIterator> GetLine(size_t i) const;

	LUX_API float GetLineWidth(size_t i) const;
	LUX_API math::Dimension2F GetDimension() const;

private:
	bool m_Rebreak;

	// Cached data
	WeakRef<Font> m_Font;
	bool m_Wrap;
	math::Dimension2F m_TextBoxSize;
	gui::FontRenderSettings m_FontSettings;

	// Generated data
	core::Array<core::Range<String::ConstIterator>> m_BrokenText;
	core::Array<float> m_LineSizes;
	math::Dimension2F m_TextDim;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_GUI_TEXT_CONTAINER_H
