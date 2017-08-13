#ifndef INCLUDED_GUI_SKIN_H
#define INCLUDED_GUI_SKIN_H
#include "GUIRenderer.h"
#include "video/SpriteBank.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{

enum class EGUIColor
{
	Bright,
	Foreground,
	Mid,
	Background,
	Dark,
	Shadow,

	Base,
	BaseDisabled,
	AlternateBase,

	Button,
	ButtonPressed,
	ButtonDisabled,

	ToolTipBase,
	ToolTipText,

	Text,
	DisabledText,
	BrightText,

	Highlight,
	HighlightText,
};

enum class EGUISize
{};

enum class EGUISprite
{};

struct GUISprite
{
	video::SpriteBank::Sprite sprite;
	video::Color color;
	bool loop;
};

class Element;
class Renderer;
class Skin : public ReferenceCounted
{
public:
	virtual ~Skin() {}

	void SetColor(EGUIColor color, video::Color value)
	{
		m_Colors[color] = value;
	}

	video::Color GetColor(EGUIColor color) const
	{
		auto it = m_Colors.Find(color);
		if(it == m_Colors.End())
			return video::Color(255, 64, 255); // TODO: Approximate color
		return *it;
	}

	void SetSize(EGUISize size, float value)
	{
		m_Sizes[size] = value;
	}
	float GetSize(EGUISize size) const
	{
		auto it = m_Sizes.Find(size);
		if(it == m_Sizes.End())
			return 0;
		return *it;
	}

	void SetSprite(EGUISprite sprite, const GUISprite& guiSprite)
	{
		m_Sprites[sprite] = guiSprite;
	}
	bool GetSprite(EGUISprite sprite, GUISprite& outSprite) const
	{
		auto it = m_Sprites.Find(sprite);
		if(it == m_Sprites.End())
			return false;
		outSprite = *it;
		return true;
	}

	virtual void DrawButtonPaneNormal(Renderer* r, Element* element, const math::RectF& rect, const math::RectF* clip = nullptr)
	{
		DrawPane(r, element, rect, GetColor(EGUIColor::Button), clip);
	}

	virtual void DrawButtonPanePressed(Renderer* r, Element* element, const math::RectF& rect, const math::RectF* clip = nullptr)
	{
		DrawPane(r, element, rect, GetColor(EGUIColor::ButtonPressed), clip);
	}
	virtual void DrawWindowBackground(Renderer* r, Element* element, const math::RectF& rect, const math::RectF* clip = nullptr)
	{
		DrawPane(r, element, rect, GetColor(EGUIColor::Background), clip);
	}

	virtual void DrawPane(Renderer* r, Element* element, const math::RectF& rect, video::Color color, const math::RectF* clip = nullptr)
	{
		LUX_UNUSED(element);
		auto realRect = rect;
		if(clip)
			realRect.FitInto(*clip);
		r->DrawRectangle(realRect, color);
	}

	virtual video::SpriteBank* GetSpriteBank() const
	{
		return m_Bank;
	}

	virtual void SetSpriteBank(video::SpriteBank* bank)
	{
		m_Bank = bank;
	}

	virtual void SetDefaultFont(gui::Font* font)
	{
		m_Font = font;
	}

	virtual gui::Font* GetDefaultFont()
	{
		return m_Font;
	}

protected:
	StrongRef<gui::Font> m_Font;
	StrongRef<video::SpriteBank> m_Bank;
	core::HashMap<EGUISize, float> m_Sizes;
	core::HashMap<EGUISprite, GUISprite> m_Sprites;
	core::HashMap<EGUIColor, video::Color> m_Colors;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_SKIN_H