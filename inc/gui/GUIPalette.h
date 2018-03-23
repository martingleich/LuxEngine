#ifndef INCLUDED_LUX_GUI_PALETTE_H
#define INCLUDED_LUX_GUI_PALETTE_H
#include "video/Color.h"
#include "gui/GUIEnums.h"

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
		Data() :
			colorFlags(0),
			refCount(1)
		{
		}

		video::Color colors[COLOR_COUNT];
		u32 colorFlags;
		int refCount;
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
		for(int i = 0; i < COLOR_COUNT; ++i)
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

	void EnsureDataCopy()
	{
		if(m_Data->refCount > 1)
			m_Data = LUX_NEW(Data)(*m_Data);
	}

private:
	Data* m_Data;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_PALETTE_H