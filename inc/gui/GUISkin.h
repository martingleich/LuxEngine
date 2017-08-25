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
	Enabled = 1,
	Highlighted = 2,
	Focused = 4,

	Disabled = 0
};
}
DECLARE_FLAG_CLASS(gui::EGUIState);
namespace gui
{

enum class EGUIPrimitive
{
	Pane,

	Button,
	ButtonPressed,

	Window,
	WindowBase,

	Frame,

	CheckBoxEmpty,
	CheckBoxChecked,

	RadioButtonEmpty,
	RadioButtonChecked,

	TextArea,
	StaticText,

	Tooltip,
};

class Element;
class Renderer;

class Skin : public ReferenceCounted
{
public:
	Skin()
	{
		enabledColor = video::Colorf(1, 1, 1, 1);
		disabledColor = video::Colorf(0.6f, 0.6f, 0.6f, 1.0f);
		textColor = video::Colorf(0, 0, 0);
	}

	virtual ~Skin() {}

	virtual void DrawPrimitive(
		Renderer* r,
		EGUIState state,
		EGUIPrimitive prim,
		const math::RectF& rect,
		const video::Colorf& color,
		const math::RectF* clip = nullptr) = 0;

	virtual void DrawCursor(
		Renderer* r,
		ECursorState state,
		bool pressed,
		const math::Vector2F& position) = 0;

	StrongRef<Font> defaultFont;
	video::Colorf enabledColor;
	video::Colorf disabledColor;
	video::Colorf textColor;
	video::Colorf disabledTextColor;

	math::Vector2F buttonPressedTextOffset;
};

class Skin3D : public Skin
{
public:
	Skin3D()
	{
		window = video::Color(180, 180, 180);
		face = video::Color(210, 210, 210);
		darkShadow = 0.2f;
		shadow = 0.6f;
		light = 1.4f;
		highLight = 1.7f;

		buttonPressedTextOffset = math::Vector2F(1, 1);
	}

	void DrawPrimitive(
		Renderer* r,
		EGUIState state,
		EGUIPrimitive prim,
		const math::RectF& rect,
		const video::Colorf& color,
		const math::RectF* clip = nullptr)
	{
		auto realColor = color;
		if(TestFlag(state, EGUIState::Enabled))
			realColor *= enabledColor;
		if(state == EGUIState::Disabled)
			realColor *= disabledColor;

		switch(prim) {
		case EGUIPrimitive::Button:
			Draw3DButtonPane(r, false, rect, realColor, clip);
			break;
		case EGUIPrimitive::ButtonPressed:
			Draw3DButtonPane(r, true, rect, realColor, clip);
			break;
		case EGUIPrimitive::WindowBase:
			DrawWindowBase(r, rect, realColor, clip);
			break;
		default:
			r->DrawRectangle(rect, (realColor*face).ToHex(), clip);
			break;
		}
	}

	void DrawCursor(
		Renderer* r,
		ECursorState state,
		bool pressed,
		const math::Vector2F& position)
	{
		LUX_UNUSED(state);
		video::Color color = video::Color(200, 0, 0);
		if(pressed)
			color = video::Color(255,10,10);
		r->DrawTriangle(
			position,
			position + math::Vector2F(0.0f, 20.0f),
			position + math::Vector2F(14.0f, 14.0f), color, nullptr);
	}

private:
	void DrawWindowBase(
		Renderer* r,
		const math::RectF& rect,
		const video::Colorf& color,
		const math::RectF* clip)
	{
		r->DrawRectangle(rect, (color*window).ToHex(), clip);
	}

	void Draw3DButtonPane(
		Renderer* r,
		bool pressed,
		const math::RectF& rect,
		const video::Colorf& color,
		const math::RectF* clip)
	{
		auto rct = rect;
		auto baseColor = face * color;
		if(pressed) {
			r->DrawRectangle(rct, (baseColor*highLight).ToHex(), clip);
			rct.right -= 1;
			rct.bottom -= 1;
			r->DrawRectangle(rct, (baseColor*darkShadow).ToHex(), clip);
			rct.left += 1;
			rct.top += 1;
			r->DrawRectangle(rct, (baseColor*shadow).ToHex(), clip);
			rct.left += 1;
			rct.top += 1;
			r->DrawRectangle(rct, (baseColor).ToHex(), clip);
		} else {
			r->DrawRectangle(rct, (baseColor*darkShadow).ToHex(), clip);
			rct.right -= 1;
			rct.bottom -= 1;
			r->DrawRectangle(rct, (baseColor*highLight).ToHex(), clip);
			rct.left += 1;
			rct.top += 1;
			r->DrawRectangle(rct, (baseColor*shadow).ToHex(), clip);
			rct.right -= 1;
			rct.bottom -= 1;
			r->DrawRectangle(rct, (baseColor).ToHex(), clip);
		}
	}

public:
	float darkShadow;
	float shadow;
	video::Colorf face;
	float light;
	float highLight;

	video::Colorf window;
	video::Colorf baseEditable;
};

} // namespace gui
} // namespace lux


#endif // #ifndef INCLUDED_GUI_SKIN_H