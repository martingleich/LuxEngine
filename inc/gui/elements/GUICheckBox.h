#ifndef INCLUDED_LUX_GUI_CHECK_BOX_H
#define INCLUDED_LUX_GUI_CHECK_BOX_H
#include "gui/elements/GUIAbstractButton.h"
#include "gui/GUITextContainer.h"

namespace lux
{
namespace gui
{

class CheckBox : public AbstractButton
{
	LX_REFERABLE_MEMBERS_API(CheckBox, LUX_API);
public:
	LUX_API CheckBox();
	LUX_API ~CheckBox();

	LUX_API void Paint(Renderer* renderer);
	LUX_API void SetText(const core::String& text);
	LUX_API const core::String& GetText() const;
	LUX_API bool IsChecked() const;
	LUX_API virtual void SetChecked(bool b);
	LUX_API EGUIStateFlag GetState() const;

protected:
	LUX_API void OnClick();

public:
	core::Signal<bool> onStateChange;

protected:
	TextContainer m_Text;
	bool m_IsChecked;
};
} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUI_CHECK_BOX_H

