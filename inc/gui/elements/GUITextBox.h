#ifndef INCLUDED_GUI_TEXT_BOX_H
#define INCLUDED_GUI_TEXT_BOX_H
#include "gui/GUIElement.h"
#include "gui/GUITextContainer.h"
#include "events/lxSignal.h"

namespace lux
{
namespace gui
{

class TextBox : public Element
{
public:
	LX_REFERABLE_MEMBERS_API(LUX_API);

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
	void SetCursor(size_t pos);

	size_t BPos() const;
	size_t WPos() const;

public:
	event::Signal<const core::String&> onTextChange;

private:
	gui::TextContainer m_Container;
	size_t m_Caret = 0;
	float m_Time = 0;

	float m_Offset = 0;
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_TEXT_BOX_H