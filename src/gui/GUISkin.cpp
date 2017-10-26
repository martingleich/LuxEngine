#include "gui/GUISkin.h"
#include "gui/GUIElement.h"

namespace lux
{
namespace gui
{

Palette::Data Palette::DEFAULT_DATA;

Skin3D::Skin3D() :
	shadow(0.2f),
	light(1.2f)
{
	SetPropertyV("lux.gui.Slider.thumbSize", math::Vector2F(10, 30));
}

void Skin3D::DrawCursor(
	Renderer* r,
	ECursorState state,
	bool pressed,
	const math::Vector2F& position)
{
	LUX_UNUSED(state);
	video::Color color = video::Color(200, 0, 0);
	if(pressed)
		color = video::Color(255, 10, 10);
	math::AngleF rot = math::AngleF::Degree(5.0f);
	math::AngleF a = math::AngleF::Degree(45.0f);
	float s1 = 25.0f;
	float s2 = 20.0f;
	math::Vector2F p1(0, 0);
	math::Vector2F p2 = math::Vector2F::BuildFromPolar(rot, s1);
	p2.x = std::floor(p2.x); p2.y = std::floor(p2.y);
	math::Vector2F p3 = math::Vector2F::BuildFromPolar(rot + a / 2, s2);
	p3.x = std::floor(p3.x); p3.y = std::floor(p3.y);
	math::Vector2F p4 = math::Vector2F::BuildFromPolar(rot + a, s1);
	p4.x = std::floor(p4.x); p4.y = std::floor(p4.y);
	r->DrawTriangle(
		position + p1,
		position + p2,
		position + p3, color.Scaled(0.7f), nullptr);
	r->DrawTriangle(
		position + p1,
		position + p3,
		position + p4, color, nullptr);
}

void Skin3D::DrawControl(
	Renderer* r,
	Element* elem,
	const math::RectF& rect,
	EGUIControl control,
	EGUIState state,
	const PaintOptions& data)
{
	LUX_UNUSED(elem);
	Palette::EColorGroup paletteState = TestFlag(state, EGUIState::Enabled) ? Palette::EColorGroup::Enabled : Palette::EColorGroup::Disabled;
	switch(control) {
	case EGUIControl::Button:
		DrawPane(r, TestFlag(state, EGUIState::Sunken), rect, data.palette.GetColor(paletteState, Palette::EColorRole::Window));
		break;
	case EGUIControl::SliderBase:
		DrawSliderBase(
			r,
			rect,
			(const SliderPaintOptions&)data,
			data.palette.GetColor(paletteState, Palette::EColorRole::WindowBackground));
		break;
	case EGUIControl::SliderThumb:
		DrawPane(r, TestFlag(state, EGUIState::Sunken), rect, data.palette.GetColor(paletteState, Palette::EColorRole::Window));
		break;
	default:
		r->DrawRectangle(rect, data.palette.GetColor(paletteState, Palette::EColorRole::Window));
	}
}

void Skin3D::DrawSliderBase(
	Renderer* r,
	const math::RectF& rect,
	const SliderPaintOptions& data,
	const video::Color& color)
{
	float lineWidth = 3.0f;
	if(data.isHorizontal) {
		float thumbWidth = data.thumbRect.GetWidth();
		float linePos = rect.GetCenter().y;
		math::RectF line(
			rect.left + thumbWidth / 2, linePos - lineWidth / 2,
			rect.right - thumbWidth / 2, linePos + lineWidth / 2);
		DrawPane(r, false, line, color);
	} else {
		float thumbHeight = data.thumbRect.GetHeight();
		float linePos = rect.GetCenter().x;
		math::RectF line(
			linePos + lineWidth / 2, rect.top + thumbHeight / 2,
			linePos - lineWidth / 2, rect.bottom - thumbHeight / 2);
		DrawPane(r, false, line, color);
	}
}

void Skin3D::DrawPane(
	Renderer* r,
	bool sunken,
	const math::RectF& rect,
	const video::Color& color)
{
	auto rct = rect;
	auto baseColor = color;
	float medShadow = math::Lerp(shadow, light, 0.25f);
	if(sunken) {
		r->DrawRectangle(rct, baseColor.Scaled(light));
		rct.right -= 1;
		rct.bottom -= 1;
		r->DrawRectangle(rct, baseColor.ScaledA(shadow));
		rct.left += 1;
		rct.top += 1;
		r->DrawRectangle(rct, baseColor.ScaledA(medShadow));
		rct.left += 1;
		rct.top += 1;
		r->DrawRectangle(rct, baseColor);
	} else {
		rct.right += 1;
		rct.bottom += 1;
		r->DrawRectangle(rct, baseColor.ScaledA(shadow));
		rct.right -= 1;
		rct.bottom -= 1;
		r->DrawRectangle(rct, baseColor.Scaled(light));
		rct.left += 1;
		rct.top += 1;
		r->DrawRectangle(rct, baseColor.ScaledA(medShadow));
		rct.right -= 1;
		rct.bottom -= 1;
		r->DrawRectangle(rct, baseColor);
	}
}

} // namespace gui
} // namespace lux