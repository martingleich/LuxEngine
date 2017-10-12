#ifndef INCLUDED_GUI_STATIC_TEXT_H
#define INCLUDED_GUI_STATIC_TEXT_H
#include "gui/GUIElement.h"
#include "gui/Font.h"

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

	LUX_API void SetDrawBackground(bool draw);
	LUX_API bool GetDrawBackground() const;

	LUX_API void SetWordWrap(bool wrap);
	LUX_API bool GetWordWrap() const;

	LUX_API void FitSizeToText();
	LUX_API void SetFitSizeToText(bool fit);
	LUX_API bool GetFitSizeToText() const;

	LUX_API void SetClipTextInside(bool clip);
	LUX_API bool GetClipTextInside() const;

	LUX_API void SetText(const String& text);
	LUX_API core::Name GetReferableType() const;

protected:
	void EnsureBrokenText();
	void OnInnerRectChange();

protected:
	WeakRef<Font> m_LastBrokenFont;
	core::Array<String> m_BrokenText;
	float m_TextHeight;
	float m_TextWidth;

	bool m_RebreakText;
	bool m_WordWrap;
	bool m_DrawBackground;
	bool m_OverwriteColor;
	bool m_ClipTextInside;
	bool m_FitSizeToText;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_STATIC_TEXT_H
