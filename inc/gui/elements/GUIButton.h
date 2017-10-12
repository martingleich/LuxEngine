#ifndef INCLUDED_GUI_BUTTON_H
#define INCLUDED_GUI_BUTTON_H
#include "gui/elements/GUIAbstractButton.h"
#include "video/Color.h"

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
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_BUTTON_H