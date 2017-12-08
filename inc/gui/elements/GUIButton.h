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
	LX_REFERABLE_MEMBERS_API(LUX_API);

	LUX_API Button(bool isSwitchButton);
	LUX_API ~Button();
	LUX_API void Paint(Renderer* r);
	LUX_API void SetText(const core::String& text);
	LUX_API const core::String& GetText() const;

private:
	TextContainer m_Text;
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_BUTTON_H