#ifndef INCLUDED_LUX_GUI_SKIN_3D_H
#define INCLUDED_LUX_GUI_SKIN_3D_H
#include "gui/GUISkin.h"

namespace lux
{
namespace gui
{

class Skin3D : public Skin
{
	LX_REFERABLE_MEMBERS_API(Skin3D, LUX_API);
public:
	LUX_API Skin3D();

	LUX_API void DrawCursor(
		Renderer* r,
		ECursorState state,
		bool pressed,
		const math::Vector2F& position,
		float animTime);

	LUX_API void DrawControl(
		Renderer* r,
		Element* elem,
		const math::RectF& rect,
		EGUIControl control,
		EGUIState state,
		const PaintOptions& data);

	LUX_API void DrawFocus(
		Renderer* r,
		Element* elem);

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
	void DrawCheckBox(
		Renderer* r,
		bool checked,
		const math::RectF& rect,
		bool enabled);
	void DrawRadioButton(
		Renderer* r,
		bool checked,
		const math::RectF& rect,
		bool enabled);
	void DrawNormalCursor(
		Renderer* r,
		bool pressed,
		const math::Vector2F& position,
		float animTime);
	void DrawBeamCursor(
		Renderer* r,
		bool pressed,
		const math::Vector2F& position,
		float animTime);
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_SKIN_3D_H