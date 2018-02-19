#ifndef INCLUDED_GUI_RADIO_BUTTON_H
#define INCLUDED_GUI_RADIO_BUTTON_H
#include "gui/elements/GUIAbstractButton.h"
#include "gui/GUITextContainer.h"

namespace lux
{
namespace gui
{

class RadioButton : public AbstractButton
{
	LX_REFERABLE_MEMBERS_API(RadioButton, LUX_API);

	class Group
	{
	public:
		Group(RadioButton* rb) :
			button(rb)
		{
		}

		void Add(RadioButton* b)
		{
			lxAssert(button);

			if(b->m_Next != b)
				Group(b).Remove(b);
			b->m_Next = button->m_Next;
			button->m_Next = b;
		}

		void Remove(RadioButton* b)
		{
			RadioButton* start = button;
			RadioButton* prev = button;
			RadioButton* cur = button->m_Next;
			do {
				if(cur == b) {
					prev->m_Next = cur->m_Next;
					if(button == b)
						button = prev;
					return;
				}
				prev = cur;
				cur = cur->m_Next;
			} while(prev != start);
		}

	private:
		RadioButton* button;
	};
	friend class Group;

public:
	LUX_API RadioButton();
	LUX_API ~RadioButton();

	LUX_API void Paint(Renderer* renderer);

	LUX_API bool IsChecked() const;
	LUX_API void SetChecked(bool b);

	LUX_API void SetText(const core::String& str);
	LUX_API const core::String& GetText() const;

	LUX_API EGUIState GetState() const;

	LUX_API RadioButton* GetRadioGroupChecked();
	LUX_API void SetRadioGroup(RadioButton* group);

public:
	event::Signal<bool> onStateChange;

protected:
	LUX_API void OnClick();

protected:
	TextContainer m_Text;
	bool m_IsChecked;

	RadioButton* m_Next;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_RADIO_BUTTON_H