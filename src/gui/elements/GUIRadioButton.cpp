#include "gui/elements/GUIRadioButton.h"
#include "gui/GUISkin.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::RadioButton, "lux.gui.RadioButton");

namespace lux
{
namespace gui
{

static const math::Dimension2F DEFAULT_RADIOBUTTON_SIZE(17, 17);

RadioButton::RadioButton() :
	AbstractButton(true)
{
	m_Next = this;

	onStateChange.SetConnectEvent([this](const core::SignalFunc<bool>& f) { f.Call(m_IsChecked); });
	SetMinSize(DEFAULT_RADIOBUTTON_SIZE);
	SetAlignment(CombineFlags(EAlign::VCenter, EAlign::HLeft));
	m_IsChecked = true;
}

RadioButton::~RadioButton()
{
	Group(this).Remove(this);
}

void RadioButton::Paint(Renderer* renderer)
{
	auto align = GetAlignment();
	auto final = GetFinalInnerRect();
	auto font = GetFont();
	auto skin = GetSkin();
	auto palette = GetFinalPalette();
	auto checkBoxSize = skin->GetPropertyDim("lux.gui.RadioButton.size", DEFAULT_RADIOBUTTON_SIZE);
	auto checkBox = AlignInnerRect<float>(
		checkBoxSize.width, checkBoxSize.height,
		final,
		align);
	float textBoxWidth;
	float textBoxHeight;
	if(TestFlag(align, EAlign::HCenter))
		textBoxWidth = final.GetWidth();
	else
		textBoxWidth = final.GetWidth() - checkBoxSize.width;
	if(TestFlag(align, EAlign::VCenter))
		textBoxHeight = final.GetHeight();
	else
		textBoxHeight = final.GetHeight() - checkBoxSize.height;

	auto textBox = AlignOuterRect<float>(
		textBoxWidth, textBoxHeight,
		checkBox,
		FlipAlign(align, true, true));
	textBox.left += 4;
	textBox.right += 4;

	gui::PaintOptions po;
	po.palette = palette;
	skin->DrawControl(renderer,
		this,
		checkBox,
		EGUIControl::RadioButton,
		GetState(),
		po);

	gui::FontRenderSettings settings;
	settings.color = palette.GetWindowText(GetState());
	m_Text.Render(renderer, font, settings, false, align, textBox);
}

bool RadioButton::IsChecked() const
{
	return m_IsChecked;
}

void RadioButton::SetChecked(bool b)
{
	if(!IsChecked() && b) {
		for(auto it = m_Next; it != this; it = it->m_Next)
			it->SetChecked(false);
	}
	if(b != IsChecked()) {
		m_IsChecked = b;
		onStateChange.Broadcast(b);
	}
}

void RadioButton::SetText(const core::String& str)
{
	m_Text.SetText(str);
}
const core::String& RadioButton::GetText() const
{
	return m_Text.GetText();
}

EGUIStateFlag RadioButton::GetState() const
{
	EGUIStateFlag state = Element::GetState();
	if(m_IsChecked)
		SetFlag(state, EGUIStateFlag::Sunken);
	return state;
}

void RadioButton::OnClick()
{
	if(!IsChecked())
		SetChecked(true);
	AbstractButton::OnClick();
}

void RadioButton::SetRadioGroup(RadioButton* group)
{
	Group cur(this);
	cur.Remove(this);
	Group next(group);
	next.Add(this);

	if(IsChecked()) {
		for(auto it = m_Next; it != this; it = it->m_Next)
			it->SetChecked(false);
	}
}

RadioButton* RadioButton::GetRadioGroupChecked()
{
	if(IsChecked())
		return this;

	for(auto it = m_Next; it != this; it = it->m_Next) {
		if(it->IsChecked())
			return it;
	}
	return nullptr;
}

} // namespace gui
} // namespace lux
