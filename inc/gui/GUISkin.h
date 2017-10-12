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
		Window = 0,
		WindowText = 1,

		Base = 2,
		BaseText = 3,

		BaseHightlight = 4,
		BaseHightlightText = 5,
	};

	enum class EColorGroup
	{
		Enabled = 0,
		Disabled = 1,
	};

	static const u32 COLOR_COUNT = 12;

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
		if(!IsSet(state, role) && state == EColorGroup::Disabled && IsSet(EColorGroup::Enabled, role)) {
			return 0.5f*GetColor(state, role);
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
		static const u32 BIT_MASK = (((1 << COLOR_COUNT)-1)<<1)+1;
		if((m_Data->colorFlags & BIT_MASK) == BIT_MASK)
			return;

		EnsureDataCopy();
		for(u32 i = 0; i < COLOR_COUNT; ++i) {
			if((m_Data->colorFlags & (1 << i)) == 0)
				m_Data->colors[i] = base.m_Data->colors[i];
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

	Tooltip,
};

struct PaintOptions
{
	Palette palette;
};

class Skin : public ReferenceCounted
{
public:
	Skin()
	{
		m_SunkenOffset = math::Vector2F(1, 1);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Enabled, Palette::EColorRole::WindowText, video::Color::Black);
		m_DefaultPalette.SetColor(Palette::EColorGroup::Disabled, Palette::EColorRole::WindowText, video::Color::LightGray);
		m_DefaultPalette.SetColor(Palette::EColorRole::BaseText, video::Color::Black);
		m_DefaultPalette.SetColor(Palette::EColorRole::Base, video::Color::White);
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

	virtual const Palette& GetDefaultPalette() const
	{
		return m_DefaultPalette;
	}

	virtual const math::Vector2F& GetSunkenOffset() const { return m_SunkenOffset; }

private:
	StrongRef<Font> m_DefaultFont;
	Palette m_DefaultPalette;
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
	void Draw3DButtonPane(
		Renderer* r,
		bool pressed,
		const math::RectF& rect,
		const video::Color& color);
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_SKIN_H