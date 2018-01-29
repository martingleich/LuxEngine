#ifndef INCLUDED_GUI_ENUMS_H
#define INCLUDED_GUI_ENUMS_H
#include "core/LuxBase.h"

namespace lux
{
namespace gui
{

enum class EGUIState
{
	None = 0,
	Enabled = 0x1,
	Selected = 0x2,
	Focused = 0x4,
	On = 0x8,
	Off = 0x10,
	Hovered = 0x20,
	Clicked = 0x40,
	Editing = 0x80,

	Sunken = 0x100,
	Raised = 0x200,
};
}
DECLARE_FLAG_CLASS(gui::EGUIState);
}

#endif // #ifndef INCLUDED_GUI_ENUMS_H