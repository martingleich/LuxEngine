#ifndef INCLUDED_GUI_SKIN_H
#define INCLUDED_GUI_SKIN_H
#include "gui/GUIRenderer.h"
#include "gui/Cursor.h"
#include "gui/Font.h"

namespace lux
{
namespace gui
{

enum class EGUIState
{
	None = 0,
	Enabled = 0x1,
	Selected = 0x2,
	Focused = 0x4,
	On = 0x8,
	Off = 0x10,
	Hovered = 0x20,
	Clicked = 0x40,
	Editing = 0x80,
	Sunken = 0x100,
	Raised = 0x200,
};
}
DECLARE_FLAG_CLASS(gui::EGUIState);
namespace gui
{
class Element;
class Renderer;

class Palette
{
public:
	enum class EColorRole
	{
		WindowBackground,
		Window,
		WindowText,

		Base,
		BaseText,

		BaseHightlight,
		BaseHightlightText,
	};

	enum class EColorGroup
	{
		Enabled = 0,
		Disabled = 1,
	};

	static const u32 COLOR_COUNT = 14;

private:
	struct Data
	{
		Data() :
			colorFlags(0),
			refCount(1)
		{
		}

		video::Color colors[COLOR_COUNT];
		u32 colorFlags;
		size_t refCount;
	};

	LUX_API static Data DEFAULT_DATA;

public:
	Palette()
	{
		m_Data = &DEFAULT_DATA;
		m_Data->refCount++;
	}

	Palette(video::Color color) :
		Palette()
	{
		for(u32 i = 0; i < COLOR_COUNT; ++i)
			SetColor(i, color);
	}

	Palette(const Palette& other)
	{
		m_Data = other.m_Data;
		++m_Data->refCount;
	}

	Palette(const Palette& other, const Palette& base) :
		Palette(other)
	{
		Merge(base);
	}

	~Palette()
	{
		--m_Data->refCount;
		if(!m_Data->refCount)
			LUX_FREE(m_Data);
	}

	Palette& operator=(const Palette& other)
	{
		this->~Palette();
		m_Data = other.m_Data;
		++m_Data->refCount;
		return *this;
	}

	bool IsSet(EColorGroup state, EColorRole role) const
	{
		return (m_Data->colorFlags & (1 << GetId(state, role))) != 0;
	}

	video::Color GetColor(EColorGroup state, EColorRole role) const
	{
		if(!IsSet(state, role)) {
			if(state == EColorGroup::Disabled && IsSet(EColorGroup::Enabled, role))
				return 0.5f*GetColor(state, role);
			if(role == EColorRole::BaseText)
				return GetColor(state, EColorRole::WindowText);
			if(role == EColorRole::BaseHightlight)
				return GetColor(state, EColorRole::Base).GetInverted();
			if(role == EColorRole::BaseHightlightText)
				return GetColor(state, EColorRole::BaseText).GetInverted();
			lxAssertNeverReach("Can't get not set color");
			return video::Color::Black;
		} else {
			return m_Data->colors[GetId(state, role)];
		}
	}

	video::Color GetWindow(EGUIState state) const { return TestFlag(state, EGUIState::Enabled) ? GetWindow() : GetDisabledWindow(); }
	video::Color GetWindowText(EGUIState state) const { return TestFlag(state, EGUIState::Enabled) ? GetWindowText() : GetDisabledWindowText(); }
	video::Color GetWindow() const { return GetColor(EColorGroup::Enabled, EColorRole::Window); }
	video::Color GetDisabledWindow() const { return GetColor(EColorGroup::Disabled, EColorRole::Window); }
	video::Color GetWindowText() const { return GetColor(EColorGroup::Enabled, EColorRole::WindowText); }
	video::Color GetDisabledWindowText() const { return GetColor(EColorGroup::Disabled, EColorRole::WindowText); }

	void SetColor(EColorGroup state, EColorRole role, video::Color color)
	{
		EnsureDataCopy();
		SetColor(GetId(state, role), color);
	}
	void SetColor(EColorRole role, video::Color color)
	{
		SetColor(EColorGroup::Disabled, role, color);
		SetColor(EColorGroup::Enabled, role, color);
	}

	void SetColor(u32 id, video::Color color)
	{
		EnsureDataCopy();
		m_Data->colors[id] = color;
		m_Data->colorFlags |= 1 << id;
	}

	void ClearColor(EColorGroup state, EColorRole role)
	{
		EnsureDataCopy();
		m_Data->colorFlags &= ~(1 << GetId(state, role));
	}

	bool IsEmpty() const
	{
		return m_Data->colorFlags == 0;
	}

	void Merge(const Palette& base)
	{
		if(IsEmpty()) {
			*this = base;
			return;
		}
		static const u32 BIT_MASK = (((1 << COLOR_COUNT) - 1) << 1) + 1;
		if((m_Data->colorFlags & BIT_MASK) == BIT_MASK)
			return;

		EnsureDataCopy();
		for(u32 i = 0; i < COLOR_COUNT; ++i) {
			if((m_Data->colorFlags & (1 << i)) == 0) {
				m_Data->colors[i] = base.m_Data->colors[i];
				m_Data->colorFlags |= (1 << i);
			}
		}
	}

private:
	u32 GetId(EColorGroup state, EColorRole role) const
	{
		return ((u32)state) * 6 + (u32)role;
	}

	void EnsureDataCopy()
	{
		if(m_Data->refCount > 1)
			m_Data = LUX_NEW(Data)(*m_Data);
	}

private:
	Data* m_Data;
};

enum class EGUIControl
{
	Button,
	CheckBox,
	RadioButton,

	TextArea,
	StaticText,

	SliderBase,
	SliderThumb,

	Tooltip,
};

struct PaintOptions
{
	Palette palette;
};

struct SliderPaintOptions : PaintOptions
{
	bool isHorizontal;
	math::RectF thumbRect;
};

class Skin : public ReferenceCounted
{
public:
	Skin()
	{
		m_SunkenOffset = math::Vector2F(1, 1);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Enabled, Palette::EColorRole::WindowText, video::Color::Black);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Disabled, Palette::EColorRole::WindowText, video::Color::LightGray);
		m_DefaultPalette.SetColor(Palette::EColorRole::Base, video::Color::White);
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
		const math::Vector2F& position) = 0;

	virtual void DrawControl(
		Renderer* r,
		Element* elem,
		const math::RectF& rect,
		EGUIControl control,
		EGUIState state,
		const PaintOptions& data) = 0;

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
		auto it = m_DefaultPalettes.Find(name);
		if(it == m_DefaultPalettes.End())
			return m_DefaultPalette;
		else
			return *it;
	}

	virtual const math::Vector2F& GetSunkenOffset() const { return m_SunkenOffset; }

	float GetPropertyF(const String& prop, float default=0.0f)
	{
		auto it = m_PropsF.Find(prop);
		if(it == m_PropsF.End())
			return default;
		return *it;
	}
	void SetPropertyF(const String& prop, float v)
	{
		m_PropsF[prop] = v;
	}

	math::Vector2F GetPropertyV(const String& prop, math::Vector2F default=math::Vector2F(0))
	{
		auto it = m_PropsV.Find(prop);
		if(it == m_PropsV.End())
			return default;
		return *it;
	}
	void SetPropertyV(const String& prop, math::Vector2F v)
	{
		m_PropsV[prop] = v;
	}

private:
	StrongRef<Font> m_DefaultFont;
	Palette m_DefaultPalette;
	core::HashMap<core::Name, Palette> m_DefaultPalettes;
	core::HashMap<String, float> m_PropsF;
	core::HashMap<String, math::Vector2F> m_PropsV;
	math::Vector2F m_SunkenOffset;
};

class Skin3D : public Skin
{
public:
	float shadow;
	float light;

public:
	LUX_API Skin3D();

	LUX_API void DrawCursor(
		Renderer* r,
		ECursorState state,
		bool pressed,
		const math::Vector2F& position);

	LUX_API void DrawControl(
		Renderer* r,
		Element* elem,
		const math::RectF& rect,
		EGUIControl control,
		EGUIState state,
		const PaintOptions& data);

private:
	void DrawSliderBase(
		Renderer* r,
		const math::RectF& rect,
		const SliderPaintOptions& data,
		const video::Color& color);
	void DrawPane(
		Renderer* r,
		bool sunken,
		const math::RectF& rect,
		const video::Color& color);
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_SKIN_H