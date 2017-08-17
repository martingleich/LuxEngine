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

	LUX_API void SetOverwriteFont(Font* f);
	LUX_API StrongRef<Font> GetOverwriteFont() const;

	LUX_API StrongRef<Font> GetActiveFont() const;

	LUX_API void SetColor(video::Color color);
	LUX_API video::Color GetColor() const;

	LUX_API void SetAlignment(Font::EAlign align);
	LUX_API Font::EAlign GetAlignment() const;

	LUX_API void SetDrawBackground(bool draw);
	LUX_API bool GetDrawBackground() const;

	LUX_API void SetWordWrap(bool wrap);
	LUX_API bool GetWordWrap() const;

	LUX_API void SetClipTextInside(bool clip);
	LUX_API bool GetClipTextInside() const;

	LUX_API void SetText(const String& text);
	LUX_API core::Name GetReferableType() const;

protected:
	void EnsureBrokenText() const;
	void OnInnerRectChange();

protected:
	Font::EAlign m_Align;
	StrongRef<Font> m_OverwriteFont;
	video::Color m_Color;
	video::Color m_Background;

	mutable WeakRef<Font> m_LastBrokenFont;
	mutable core::Array<String> m_BrokenText;

	mutable bool m_RebreakText;
	bool m_WordWrap;
	bool m_DrawBackground;
	bool m_OverwriteColor;
	bool m_ClipTextInside;
};

} // namespace gui
} // namespace lux
#endif // #ifndef INCLUDED_GUI_STATIC_TEXT_H
