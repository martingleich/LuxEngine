#include "gui/elements/GUITextBox.h"
#include "gui/GUIRenderer.h"
#include "gui/GUIEvent.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::TextBox, "lux.gui.TextBox")

namespace lux
{
namespace gui
{

TextBox::TextBox()
{
	SetAlignment(gui::EAlign::HLeft | gui::EAlign::VCenter);

	onTextChange.SetConnectEvent([this]
	(const event::SignalFunc<const core::String&>& func) {
		func.Call(m_Container.GetText()); });
}

TextBox::~TextBox()
{
}

void TextBox::Paint(Renderer* renderer, float secsPassed)
{
	m_Time += secsPassed;

	auto final = GetFinalRect();
	auto font = GetFont();
	auto palette = GetFinalPalette();
	auto state = GetState();
	auto align = GetAlignment();
	float caretToBorderDistance = 4.0f;
	gui::FontRenderSettings settings;
	settings.color = palette.GetColor(TestFlag(state, gui::EGUIState::Enabled) ?
		gui::Palette::EColorGroup::Enabled : gui::Palette::EColorGroup::Disabled, gui::Palette::EColorRole::BaseText);

	m_Container.Ensure(
		font,
		settings, false,
		math::Dimension2F(final.GetWidth(), final.GetHeight()));

	float caretOff;
	if(m_Container.GetLineCount()) {
		if(TestFlag(align, gui::EAlign::HLeft))
			caretOff = font->GetTextWidth(settings, core::SliceRange(m_Container.GetLine(0), 0, m_Caret));
		else if(TestFlag(align, gui::EAlign::HRight))
			caretOff = final.GetWidth() - font->GetTextWidth(settings, core::SliceRange(m_Container.GetLine(0), 0, m_Caret));
		else
			caretOff = final.GetWidth() / 2 - font->GetTextWidth(settings, core::SliceRange(m_Container.GetLine(0), 0, m_Caret)) / 2;
	} else {
		caretOff = 0;
	}

	float caretPos = m_Offset + caretOff;
	if(caretPos < caretToBorderDistance)
		m_Offset += caretToBorderDistance - caretPos;
	if(caretPos > final.GetWidth() - caretToBorderDistance)
		m_Offset += final.GetWidth() - caretToBorderDistance - caretPos;

	math::RectF clipRect;
	clipRect.left = final.left + caretToBorderDistance / 2;
	clipRect.right = final.right - caretToBorderDistance / 2;
	clipRect.top = final.top;
	clipRect.bottom = final.bottom;

	math::RectF renderTextBox;
	renderTextBox.left = final.left + m_Offset;
	renderTextBox.right = final.right + m_Offset;
	renderTextBox.top = final.top + 2;
	renderTextBox.bottom = final.bottom - 2;

	renderer->DrawRectangle(final,
		palette.GetColor(TestFlag(state, gui::EGUIState::Enabled) ?
			gui::Palette::EColorGroup::Enabled :
			gui::Palette::EColorGroup::Disabled, gui::Palette::EColorRole::Base));
	m_Container.Render(renderer, font, settings, false,
		align,
		renderTextBox, &clipRect);

	if(IsFocused()) {
		if(m_Time < 0.5f) {
			renderer->DrawRectangle(math::RectF(
				renderTextBox.left + caretOff,
				renderTextBox.top,
				renderTextBox.left + caretOff + 1,
				renderTextBox.bottom),
				settings.color);
		}
		if(m_Time > 1)
			m_Time -= 1;
	}
}

bool TextBox::OnKeyboardEvent(const gui::KeyboardEvent& e)
{
	if(!e.down)
		return false;
	m_Time = 0;
	switch(e.key) {
	case input::KEY_TAB:
	case input::KEY_RETURN:
		break;
	case input::KEY_HOME:
		SetCursor(0);
		return true;
	case input::KEY_END:
		SetCursor(m_Container.Text().Size());
		return true;
	case input::KEY_LEFT:
		if(m_Caret > 0) {
			if(e.ctrl)
				SetCursor(BPos());
			else
				SetCursor(m_Caret - 1);
		}
		return true;
	case input::KEY_RIGHT:
		if(m_Caret < m_Container.Text().Size()) {
			if(e.ctrl)
				SetCursor(WPos());
			else
				SetCursor(m_Caret + 1);
		}
		return true;
	case input::KEY_BACK:
		if(m_Caret > 0) {
			size_t newPos;
			if(e.ctrl)
				newPos = BPos();
			else
				newPos = m_Caret - 1;
			m_Container.Text().Remove(m_Container.Text().First() + newPos, m_Caret - newPos);
			m_Container.Rebreak();
			m_Caret = newPos;
			onTextChange.Broadcast(m_Container.GetText());
			return true;
		}
		break;
	case input::KEY_DELETE:
		if(m_Caret < m_Container.Text().Size()) {
			size_t newPos;
			if(e.ctrl)
				newPos = WPos();
			else
				newPos = m_Caret + 1;
			m_Container.Text().Remove(m_Container.Text().First() + m_Caret, newPos - m_Caret);
			m_Container.Rebreak();
			onTextChange.Broadcast(m_Container.GetText());
			return true;
		}
		break;
	default:
		for(auto it = e.character; *it; ++it)
			WriteCharacter(*it);
		if(e.character[0])
			return true;
	};
	return false;
}

bool TextBox::OnMouseEvent(const gui::MouseEvent& e)
{
	if(e.IsClick()) {
		m_Time = 0;
		if(e.leftState) {
			auto font = GetFont();
			auto final = GetFinalRect();
			gui::FontRenderSettings settings;
			auto pos = font->GetCaretFromOffset(settings, core::MakeRange(m_Container.GetText()), e.pos.x - (final.left + m_Offset));
			SetCursor(pos);
		}

		return true;
	}
	return false;
}

void TextBox::SetText(const core::String& text)
{
	m_Container.SetText(text);
	m_Container.Text().Replace("", "\n");
	m_Container.Rebreak();
	SetCursor(text.Size());

	onTextChange.Broadcast(m_Container.GetText());
}

const core::String& TextBox::GetText() const
{
	return m_Container.GetText();
}

ECursorState TextBox::GetHoverCursor() const
{
	return ECursorState::Beam;
}

void TextBox::SetTextColor(video::Color c)
{
	auto p = GetPalette();
	p.SetColor(gui::Palette::EColorGroup::Enabled, gui::Palette::EColorRole::BaseText, c);
	p.SetColor(gui::Palette::EColorGroup::Disabled, gui::Palette::EColorRole::BaseText, 0.5f*c);
	SetPalette(p);
}

void TextBox::SetBackgroundColor(video::Color c)
{
	auto p = GetPalette();
	p.SetColor(gui::Palette::EColorGroup::Enabled, gui::Palette::EColorRole::Base, c);
	p.SetColor(gui::Palette::EColorGroup::Disabled, gui::Palette::EColorRole::Base, 0.5f*c);
	SetPalette(p);
}

void TextBox::WriteCharacter(u32 character)
{
	if(character != 0) {
		if(m_Caret == m_Container.Text().Size())
			m_Container.Text().Append(character);
		else {
			core::String str;
			str.Append(character);
			m_Container.Text().Insert(
				m_Container.Text().First() + m_Caret,
				str);
		}
		m_Caret++;
		m_Container.Rebreak();
		onTextChange.Broadcast(m_Container.GetText());
	}
}

void TextBox::SetCursor(size_t pos)
{
	if(pos > m_Container.Text().Size())
		return;

	m_Caret = pos;
}

size_t TextBox::BPos() const
{
	auto text = m_Container.GetText();
	auto it = text.First() + m_Caret - 1;
	size_t count = 1;
	// Move until non space, Move until space
	while(core::IsSpace(*it) && it != text.First()) {
		--it;
		++count;
	}
	while(!core::IsSpace(*it) && it != text.First()) {
		--it;
		++count;
	}
	if(it != text.First())
		--count;
	return m_Caret - count;
}

size_t TextBox::WPos() const
{
	auto text = m_Container.GetText();
	auto it = text.First() + m_Caret + 1;
	size_t count = 1;
	// Move until space, Move until non space
	while(it != text.End() && !core::IsSpace(*it)) {
		++it;
		++count;
	}
	while(it != text.End() && core::IsSpace(*it)) {
		++it;
		++count;
	}
	return m_Caret + count;
}

} // namespace gui
} // namespace lux

