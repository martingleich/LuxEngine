#ifndef INCLUDED_LUX_GUI_SKIN_H
#define INCLUDED_LUX_GUI_SKIN_H
#include "core/Referable.h"
#include "core/lxHashMap.h"

#include "math/Rect.h"

#include "gui/GUIPalette.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{
class Element;
class Renderer;
class Cursor;

enum class EGUIControl
{
	Button,
	CheckBox,
	RadioButton,

	TextArea,
	StaticText,

	SliderBase,
	SliderThumb,

	ScrollbarBase,
	ScrollbarThumb,

	Tooltip,
};

struct PaintOptions
{
	Palette palette;
	float animTime = 0;
};

struct SliderPaintOptions : PaintOptions
{
	bool isHorizontal;
	math::RectF thumbRect;
};

class Skin : public core::Referable
{
public:
	Skin()
	{
		m_DefaultPalette.SetColor(Palette::EColorGroup::Enabled, Palette::EColorRole::WindowText, video::Color::Black);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Disabled, Palette::EColorRole::WindowText, video::Color::LightGray);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Enabled, Palette::EColorRole::Base, video::Color::White);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Disabled, Palette::EColorRole::Base, video::Color::LightGray);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Enabled, Palette::EColorRole::BaseText, video::Color::Black);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Disabled, Palette::EColorRole::BaseText, video::Color::DarkGray);
		m_DefaultPalette.SetColor(Palette::EColorRole::WindowBackground, video::Color(0xA0, 0xA0, 0xA0));
		m_DefaultPalette.SetColor(Palette::EColorRole::Window, video::Color::DarkGray);
		m_DefaultPalette.SetColor(Palette::EColorRole::BaseHightlightText, video::Color::White);
		m_DefaultPalette.SetColor(Palette::EColorRole::BaseHightlight, video::Color::Blue);
	}

	virtual ~Skin() {}

	virtual void DrawCursor(
		Renderer* r,
		ECursorState state,
		bool pressed,
		const math::Vector2F& position,
		float animTime) = 0;

	virtual void DrawControl(
		Renderer* r,
		Element* elem,
		const math::RectF& rect,
		EGUIControl control,
		EGUIStateFlag state,
		const PaintOptions& data) = 0;

	virtual void DrawFocus(
		Renderer* r,
		Element* elem) = 0;

	virtual void SetDefaultFont(Font* f)
	{
		m_DefaultFont = f;
	}
	virtual Font* GetDefaultFont() const
	{
		return m_DefaultFont;
	}

	virtual void SetDefaultPalette(const Palette& palette)
	{
		m_DefaultPalette = palette;
	}
	virtual void SetDefaultPalette(core::Name name, const Palette& palette)
	{
		auto copy = palette;
		copy.Merge(m_DefaultPalette);
		m_DefaultPalettes[name] = copy;
	}

	virtual const Palette& GetDefaultPalette(core::Name name) const
	{
		return m_DefaultPalettes.Get(name, m_DefaultPalette);
	}

	float GetPropertyFloat(const core::String& prop, float defaultValue = 0.0f)
	{
		return m_PropsF.Get(prop, defaultValue);
	}
	virtual void SetProperty(const core::String& prop, float v)
	{
		m_PropsF[prop] = v;
	}

	math::Vector2F GetPropertyVector(const core::String& prop, math::Vector2F defaultValue = math::Vector2F(0, 0))
	{
		return m_PropsV.Get(prop, defaultValue);
	}
	virtual void SetProperty(const core::String& prop, math::Vector2F v)
	{
		m_PropsV[prop] = v;
	}

	math::Dimension2F GetPropertyDim(const core::String& prop, math::Dimension2F defaultValue = math::Dimension2F(0, 0))
	{
		auto v = m_PropsV.Get(prop, math::Vector2F(defaultValue.width, defaultValue.height));
		return math::Dimension2F(v.x, v.y);
	}
	virtual void SetProperty(const core::String& prop, math::Dimension2F v)
	{
		m_PropsV[prop].Set(v.width, v.height);
	}

	core::Range<core::HashMap<core::String, float>::ConstKeyIterator>
		GetPropertiesFloat() const
	{
		return m_PropsF.Keys();
	}

	core::Range<core::HashMap<core::String, math::Vector2F>::ConstKeyIterator>
		GetPropertiesVector() const
	{
		return m_PropsV.Keys();
	}

private:
	StrongRef<Font> m_DefaultFont;
	Palette m_DefaultPalette;
	core::HashMap<core::Name, Palette> m_DefaultPalettes;
	core::HashMap<core::String, float> m_PropsF;
	core::HashMap<core::String, math::Vector2F> m_PropsV;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_SKIN_H