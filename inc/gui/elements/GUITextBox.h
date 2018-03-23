#ifndef INCLUDED_LUX_GUI_TEXT_BOX_H
#define INCLUDED_LUX_GUI_TEXT_BOX_H
#include "gui/GUIElement.h"
#include "gui/GUITextContainer.h"
#include "core/lxSignal.h"

namespace lux
{
namespace gui
{

class TextBox : public Element
{
	LX_REFERABLE_MEMBERS_API(TextBox, LUX_API);
public:
	LUX_API TextBox();
	LUX_API ~TextBox();

	LUX_API void Paint(Renderer* renderer, float secsPassed);
	LUX_API bool OnKeyboardEvent(const gui::KeyboardEvent& e);
	LUX_API bool OnMouseEvent(const gui::MouseEvent& e);

	LUX_API void SetText(const core::String& text);
	LUX_API const core::String& GetText() const;

	LUX_API ECursorState GetHoverCursor() const;

	LUX_API void SetTextColor(video::Color c);
	LUX_API void SetBackgroundColor(video::Color c);

private:
	void WriteCharacter(u32 character);
	void SetCursor(int pos);

	int BPos() const;
	int WPos() const;

public:
	core::Signal<const core::String&> onTextChange;

private:
	gui::TextContainer m_Container;
	int m_Caret = 0;
	float m_Time = 0;

	float m_Offset = 0;
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_TEXT_BOX_H