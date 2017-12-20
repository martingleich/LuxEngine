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
	SetProperty("lux.gui.Slider.thumbSize", math::Dimension2F(10, 30));
	SetProperty("lux.gui.CheckBox.size", math::Dimension2F(25, 25));
	SetProperty("lux.gui.sunkenOffset", math::Vector2F(1, 1));
}

void Skin3D::DrawCursor(
	Renderer* r,
	ECursorState state,
	bool pressed,
	const math::Vector2F& position,
	float animTime)
{
	switch(state) {
	case ECursorState::Normal:
		DrawNormalCursor(r, pressed, position, animTime);
		break;
	case ECursorState::Beam:
		DrawBeamCursor(r, pressed, position, animTime);
		break;
	default:
		DrawNormalCursor(r, pressed, position, animTime);
		break;
	}
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
	case EGUIControl::CheckBox:
		DrawCheckBox(r, TestFlag(state, EGUIState::Sunken), rect, TestFlag(state, EGUIState::Enabled));
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

void Skin3D::DrawCheckBox(
	Renderer* r,
	bool checked,
	const math::RectF& rect,
	bool enabled)
{
	video::Color back = enabled ? video::Color::White : video::Color::LightGray;
	video::Color fore = enabled ? video::Color::Black : video::Color::DarkGray;
	auto checkBox = rect;
	checkBox.left += 4;
	checkBox.right -= 4;
	checkBox.top += 4;
	checkBox.bottom -= 4;
	r->DrawRectangle(checkBox, back);

	if(checked) {
		checkBox.left += 4;
		checkBox.right -= 4;
		checkBox.top += 4;
		checkBox.bottom -= 4;
		r->DrawRectangle(checkBox, fore);
	}
}

void Skin3D::DrawNormalCursor(
	Renderer* r,
	bool pressed,
	const math::Vector2F& position,
	float animTime)
{
	LUX_UNUSED(animTime);
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

void Skin3D::DrawBeamCursor(
	Renderer* r,
	bool pressed,
	const math::Vector2F& position,
	float animTime)
{
	LUX_UNUSED(animTime);
	video::Color color = video::Color(0, 0, 0);
	float s1 = 16.0f;
	float s2 = 6.0f;
	// Top bar
	r->DrawRectangle(
		math::RectF(
			position.x - s2 / 2+1, position.y - s1 / 2 + 0,
			position.x - 0, position.y - s1 / 2 + 1),
		color);
	r->DrawRectangle(
		math::RectF(
			position.x + 1, position.y - s1 / 2 + 0,
			position.x + s2 / 2, position.y - s1 / 2 + 1),
		color);

	// Bottom bar
	r->DrawRectangle(
		math::RectF(
			position.x - s2 / 2+1, position.y + s1 / 2 + 0,
			position.x - 0, position.y + s1 / 2 + 1),
		color);
	r->DrawRectangle(
		math::RectF(
			position.x + 1, position.y + s1 / 2 + 0,
			position.x + s2 / 2, position.y + s1 / 2 + 1),
		color);

	// Vertical bar
	r->DrawRectangle(
		math::RectF(
			position.x + 0, position.y + s1 / 2,
			position.x + 1, position.y - s1 / 2 + 1),
		color);
}

} // namespace gui
} // namespace lux