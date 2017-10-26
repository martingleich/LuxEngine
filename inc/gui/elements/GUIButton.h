#ifndef INCLUDED_GUI_BUTTON_H
#define INCLUDED_GUI_BUTTON_H
#include "gui/elements/GUIAbstractButton.h"
#include "video/Color.h"
#include "gui/GUITextContainer.h"

namespace lux
{
namespace gui
{

class Button : public AbstractButton
{
public:
	LUX_API Button(bool isSwitchButton);
	LUX_API ~Button();
	LUX_API void Paint(Renderer* r);
	LUX_API core::Name GetReferableType() const;
	LUX_API void SetText(const String& text);
	LUX_API const String& GetText() const;

private:
	TextContainer m_Text;
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_BUTTON_H