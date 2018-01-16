#include "gui/elements/GUICheckBox.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::CheckBox, "lux.gui.CheckBox");

namespace lux
{
namespace gui
{

static const math::Dimension2F DEFAULT_CHECKBOX_SIZE(25, 25);
CheckBox::CheckBox() :
	AbstractButton(true)
{
	m_IsChecked = false;
	onStateChange.SetConnectEvent([this](const event::SignalFunc<bool>& f) { f.Call(m_IsChecked); });
	SetMinSize(DEFAULT_CHECKBOX_SIZE);
	SetAlignment(EAlign::VCenter | EAlign::HLeft);
}

CheckBox::~CheckBox()
{
}

void CheckBox::Paint(Renderer* renderer)
{
	auto align = GetAlignment();
	auto final = GetFinalInnerRect();
	auto font = GetFont();
	auto skin = GetSkin();
	auto palette = GetFinalPalette();
	auto checkBoxSize = skin->GetPropertyDim("lux.gui.CheckBox.size", DEFAULT_CHECKBOX_SIZE);
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

	gui::PaintOptions po;
	po.palette = palette;
	skin->DrawControl(renderer,
		this,
		checkBox,
		EGUIControl::CheckBox,
		GetState(),
		po);

	gui::FontRenderSettings settings;
	settings.color = palette.GetWindowText(GetState());
	m_Text.Render(renderer, font, settings, false, false, align, textBox);
}

void CheckBox::SetText(const core::String& text)
{
	m_Text.SetText(text);
}

const core::String& CheckBox::GetText() const
{
	return m_Text.GetText();
}

bool CheckBox::IsChecked() const
{
	return m_IsChecked;
}

void CheckBox::SetChecked(bool b)
{
	if(m_IsChecked != b) {
		m_IsChecked = b;
		onStateChange.Broadcast(m_IsChecked);
	}
}

EGUIState CheckBox::GetState() const
{
	EGUIState state = Element::GetState();
	if(m_IsChecked)
		state |= EGUIState::Sunken;
	return state;
}

void CheckBox::OnClick()
{
	SetChecked(!IsChecked());
	AbstractButton::OnClick();
}

} // namespace gui
} // namespace lux
