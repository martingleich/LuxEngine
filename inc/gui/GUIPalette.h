#ifndef INCLUDED_LUX_GUI_PALETTE_H
#define INCLUDED_LUX_GUI_PALETTE_H
#include "video/Color.h"
#include "gui/GUIEnums.h"
#include "core/lxMemory.h"

namespace lux
{
namespace gui
{

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

	static const int COLOR_COUNT = 14;

private:
	struct Data
	{
		video::Color colors[COLOR_COUNT];
		u32 colorFlags = 0;
	};

	LUX_API static core::LazyCopy<Data> DEFAULT_DATA;

public:
	Palette() :
		m_Data(DEFAULT_DATA)
	{
	}

	Palette(video::Color color) :
		Palette()
	{
		for(int i = 0; i < COLOR_COUNT; ++i)
			SetColor(i, color);
	}

	Palette(const Palette& other) = default;

	Palette(const Palette& other, const Palette& base) :
		Palette(other)
	{
		Merge(base);
	}

	~Palette() = default;

	Palette& operator=(const Palette& other) = default;

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
			return video::Color::Black;
		} else {
			return m_Data->colors[GetId(state, role)];
		}
	}

	video::Color GetWindow(EGUIStateFlag state) const { return TestFlag(state, EGUIStateFlag::Enabled) ? GetWindow() : GetDisabledWindow(); }
	video::Color GetWindowText(EGUIStateFlag state) const { return TestFlag(state, EGUIStateFlag::Enabled) ? GetWindowText() : GetDisabledWindowText(); }
	video::Color GetWindow() const { return GetColor(EColorGroup::Enabled, EColorRole::Window); }
	video::Color GetDisabledWindow() const { return GetColor(EColorGroup::Disabled, EColorRole::Window); }
	video::Color GetWindowText() const { return GetColor(EColorGroup::Enabled, EColorRole::WindowText); }
	video::Color GetDisabledWindowText() const { return GetColor(EColorGroup::Disabled, EColorRole::WindowText); }

	void SetColor(EColorGroup state, EColorRole role, video::Color color)
	{
		m_Data.ForceCopy();
		SetColor(GetId(state, role), color);
	}
	void SetColor(EColorRole role, video::Color color)
	{
		SetColor(EColorGroup::Disabled, role, color);
		SetColor(EColorGroup::Enabled, role, color);
	}

	void SetColor(u32 id, video::Color color)
	{
		m_Data.ForceCopy();
		m_Data->colors[id] = color;
		m_Data->colorFlags |= 1 << id;
	}

	void ClearColor(EColorGroup state, EColorRole role)
	{
		m_Data.ForceCopy();
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

		m_Data.ForceCopy();
		for(int i = 0; i < COLOR_COUNT; ++i) {
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

private:
	core::LazyCopy<Data> m_Data;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_PALETTE_H