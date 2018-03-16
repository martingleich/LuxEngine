#ifndef INCLUDED_GUIENVIRONMENT_H
#define INCLUDED_GUIENVIRONMENT_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "math/Vector2.h"
#include "core/lxSignal.h"
#include "gui/GUIEvent.h"

#include "gui/GUIScalarDistance.h"

namespace lux
{
namespace input
{
class Event;
}
namespace video
{
class Texture;
}
namespace gui
{
class Font;
class FontCreator;
class Element;
class Skin;
class Renderer;
class Cursor;
class Window;

class StaticText;
class Button;
class Slider;
class CheckBox;
class RadioButton;
class ImageDisplay;
class TextBox;

//! Represent all parts of the GUI
class GUIEnvironment : public ReferenceCounted
{
	class CursorControl;
public:
	LUX_API GUIEnvironment(Window* osWindow, Cursor* osCursor);
	GUIEnvironment(const GUIEnvironment&) = delete;
	LUX_API ~GUIEnvironment();

	///////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Window> GetRootElement();
	LUX_API StrongRef<Cursor> GetCursor();
	LUX_API void UseVirtualCursor(bool useVirtual);
	LUX_API void SetDrawVirtualCursor(bool draw);

	///////////////////////////////////////////////////////////////////////////

	LUX_API void Update(float secsPassed);
	LUX_API void Render();

	///////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Skin> GetSkin() const;
	LUX_API void SetSkin(Skin* skin);

	LUX_API StrongRef<Renderer> GetRenderer() const;
	LUX_API void SetRenderer(Renderer* r);

	///////////////////////////////////////////////////////////////////////////

	LUX_API void SetKeyRepeat(float timeToStart, float timeToRepeat);
	LUX_API void GetKeyRepeat(float& timeToStart, float& timeToRepeat);

	LUX_API void IgnoreKeyboard(bool b);
	LUX_API void IgnoreMouse(bool b);
	LUX_API void SendUserInputEvent(const input::Event& event);

	LUX_API StrongRef<Element> GetElementByPos(const math::Vector2F& pos);
	LUX_API bool SendElementEvent(Element* elem, const Event& event);
	LUX_API Element* GetHovered();

	LUX_API void SetFocused(Element* elem);
	LUX_API Element* GetFocused();

	///////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Element> AddElement(core::Name name, Element* parent = nullptr);
	LUX_API StrongRef<StaticText> AddStaticText(const ScalarVectorF& position, const core::String& text = core::String::EMPTY);
	LUX_API StrongRef<Button> AddButton(const ScalarVectorF& pos, const ScalarDimensionF& size, const core::String& text = core::String::EMPTY);
	LUX_API StrongRef<Slider> AddSlider(const ScalarVectorF& pos, const ScalarDistanceF& size, int min = 0, int max = 100);
	LUX_API StrongRef<Slider> AddVerticalSlider(const ScalarVectorF& pos, const ScalarDistanceF& size, int min = 0, int max = 100);
	LUX_API StrongRef<CheckBox> AddCheckBox(const ScalarVectorF& pos, const ScalarDimensionF& size, bool checked = false);
	LUX_API StrongRef<RadioButton> AddRadioButton(const ScalarVectorF& pos, const ScalarDimensionF& size, RadioButton* group = nullptr);
	LUX_API StrongRef<ImageDisplay> AddImageDisplay(const ScalarVectorF& pos, const ScalarDimensionF& size, video::Texture* img);
	LUX_API StrongRef<TextBox> AddTextBox(const ScalarVectorF& pos, const ScalarDimensionF& size);

	///////////////////////////////////////////////////////////////////////////

	//! Retrieve the font creator.
	LUX_API StrongRef<FontCreator> GetFontCreator();

	//! Get the built-in engine font.
	/**
	This font is always available, and is often used for default font values.
	*/
	LUX_API StrongRef<Font> GetBuiltInFont();

	///////////////////////////////////////////////////////////////////////////

	LUX_API void CaptureCursor(Element* elem);
	LUX_API void ReleaseCursor();
	LUX_API void OnElementRemoved(Element* elem);
	LUX_API void OnElementAdded(Element* elem);

	///////////////////////////////////////////////////////////////////////////

private:
	void OnCursorMove(const math::Vector2F& newPos);
	void OnEvent(const input::Event& event);

	void SendMouseEvent(MouseEvent& e);
	void SendKeyboardEvent(KeyboardEvent& e);

private:
	StrongRef<FontCreator> m_FontCreator;
	StrongRef<Font> m_BuiltInFont;

	StrongRef<Window> m_Root;
	StrongRef<Renderer> m_Renderer;
	StrongRef<Skin> m_Skin;

	StrongRef<Cursor> m_LuxCursor;
	StrongRef<Cursor> m_OSCursor;
	StrongRef<Window> m_OSWindow;

	StrongRef<CursorControl> m_CursorCtrl;
	Cursor* m_Cursor;

	WeakRef<Element> m_Hovered;
	WeakRef<Element> m_Focused;

	WeakRef<Element> m_Captured;

	float m_SecsPassed;
	float m_Time;

	class KeyRepeatContext
	{
	public:
		bool isActive;
		float keyRepeatTime;
		float keyRepeatStartTime;
		float timeToStart;
		float timeToRepeat;
		KeyboardEvent event;
	};
	KeyRepeatContext m_KeyRepeatContext;

	math::Vector2F m_CursorPos;
	bool m_LeftState;
	bool m_RightState;
	bool m_ControlState;
	bool m_ShiftState;

	bool m_IgnoreMouse;
	bool m_IgnoreKeyboard;

	bool m_UseVirtualCursor;
	bool m_DrawVirtualCursor;
};

} // namespace gui
} // namespace lux

#endif // INCLUDED_GUIENVIRONMENT_H
