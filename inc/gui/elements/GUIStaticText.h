#ifndef INCLUDED_GUI_STATIC_TEXT_H
#define INCLUDED_GUI_STATIC_TEXT_H
#include "gui/GUIElement.h"
#include "gui/Font.h"
#include "gui/GUITextContainer.h"

namespace lux
{
namespace gui
{

class StaticText : public Element
{
public:
	LUX_API StaticText();
	LUX_API ~StaticText();

	LUX_API void Paint(Renderer* r);

	void SetDrawBackground(bool draw) { m_DrawBackground = draw; }
	bool GetDrawBackground() const { return m_DrawBackground; }

	void SetWordWrap(bool wrap) { m_WordWrap = wrap; }
	bool GetWordWrap() const { return m_WordWrap; }

	LUX_API void FitSizeToText();
	void SetFitSizeToText(bool fit) { m_FitSizeToText = fit; }
	bool GetFitSizeToText() const { return m_FitSizeToText; }

	void SetClipTextInside(bool clip) { m_ClipTextInside = clip; }
	bool GetClipTextInside() const { return m_ClipTextInside; }

	LUX_API void SetText(const String& text);
	LUX_API core::Name GetReferableType() const;

protected:
	TextContainer m_TextContainer;

	bool m_DrawBackground;
	bool m_OverwriteColor;
	bool m_ClipTextInside;
	bool m_FitSizeToText;
	bool m_WordWrap;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_STATIC_TEXT_H
