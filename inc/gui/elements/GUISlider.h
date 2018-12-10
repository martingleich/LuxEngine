#ifndef INCLUDED_LUX_GUI_GUISLIDER_H
#define INCLUDED_LUX_GUI_GUISLIDER_H
#include "gui/GUIElement.h"
#include "core/lxSignal.h"

namespace lux
{
namespace gui
{

class Slider : public Element
{
	LX_REFERABLE_MEMBERS_API(Slider, LUX_API);
public:
	enum ESettings
	{
		Flipped = 1,
		Horizontal = 2,
		ThumbGlideToCursor = 4,
		StepOnClick = 8,
		CenterThumb = 16,
	};

public:
	LUX_API Slider();
	LUX_API ~Slider();

	LUX_API void SetSkin(Skin* skin);
	LUX_API void Paint(Renderer* renderer);
	LUX_API virtual math::Dimension2F GetThumbSize() const;
	LUX_API virtual bool IsPointOnThumb(const math::Vector2F& point) const;
	LUX_API bool OnMouseEvent(const gui::MouseEvent& e);
	LUX_API bool OnKeyboardEvent(const gui::KeyboardEvent& e);
	LUX_API void SetThumbPos(int newPos);
	LUX_API int GetThumbPos() const;
	LUX_API float GetRelThumbPos() const;
	LUX_API void SetRange(int min, int max);
	LUX_API void GetRange(int& min, int& max) const;
	LUX_API void SetStep(int step, int bigStep = 0);
	LUX_API int GetStep() const;
	LUX_API int GetBigStep() const;

	LUX_API void AddSettings(int settings);
	LUX_API void RemoveSettings(int settings);
	LUX_API int GetSettings() const;
	LUX_API void SetSettings(int settings);
	LUX_API bool IsFlipped() const;
	LUX_API void SetFlipped(bool isFlipped);
	LUX_API bool IsHorizontal() const;
	LUX_API void SetHorizontal(bool isHorizontal);

	core::Signal<int> onPosChange;
	core::Signal<> onSliderPressed;
	core::Signal<> onSliderReleased;

protected:
	LUX_API int GetThumbPos(const math::Vector2F& curPos, int offset = 0) const;
	LUX_API math::RectF GetThumbRect() const;

protected:
	math::Dimension2F m_ThumbSize;

	int m_MinValue;
	int m_MaxValue;
	int m_StepSize;
	int m_BigStep;

	int m_Pos;

	int m_GrabOffset;

	int m_Settings;

	bool m_IsPressed = false;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_GUISLIDER_H