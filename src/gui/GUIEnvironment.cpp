#include "gui/GUIEnvironment.h"
#include "core/Logger.h"

#include "input/InputSystem.h"

#include "video/VideoDriver.h"

#include "gui/Cursor.h"
#include "gui/Window.h"
#include "gui/GUISkin.h"
#include "gui/GUIElement.h"

#include "gui/FontLoader.h"
#include "gui/FontCreator.h"

#ifdef LUX_WINDOWS
#include "gui/FontCreatorWin32.h"
#endif

namespace lux
{
namespace gui
{

GUIEnvironment::GUIEnvironment(Window* osWindow, Cursor* osCursor) :
	m_OSWindow(osWindow),
	m_OSCursor(osCursor)
{
	m_OSWindow->SetEnvironment(this);

	m_Root = m_OSWindow;
	m_Cursor = m_OSCursor;

	SetSkin(LUX_NEW(Skin));

#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32);
#else
	m_FontCreator = nullptr;
#endif

	// Generate a simple skin
	m_Skin->SetColor(EGUIColor::Bright, video::Color(210, 210, 210));
	m_Skin->SetColor(EGUIColor::Foreground, video::Color(190, 190, 190));
	m_Skin->SetColor(EGUIColor::Mid, video::Color(140, 140, 140));
	m_Skin->SetColor(EGUIColor::Background, video::Color(110, 110, 110));
	m_Skin->SetColor(EGUIColor::Dark, video::Color(80, 80, 80));
	m_Skin->SetColor(EGUIColor::Shadow, video::Color(50, 50, 50));

	m_Skin->SetColor(EGUIColor::Base, video::Color(255, 255, 255));
	m_Skin->SetColor(EGUIColor::BaseDisabled, video::Color(200, 200, 200));

	m_Skin->SetColor(EGUIColor::Button, m_Skin->GetColor(EGUIColor::Foreground));
	m_Skin->SetColor(EGUIColor::ButtonPressed, m_Skin->GetColor(EGUIColor::Dark));

	m_Skin->SetColor(EGUIColor::Text, video::Color(0, 0, 0));
	m_Skin->SetColor(EGUIColor::DisabledText, video::Color(100, 100, 100));
	m_Skin->SetColor(EGUIColor::BrightText, video::Color(180, 180, 180));

	m_Skin->SetColor(EGUIColor::Highlight, video::Color(20, 20, 255));
	m_Skin->SetColor(EGUIColor::HighlightText, video::Color(255, 255, 255));

	if(m_FontCreator) {
		m_Skin->SetDefaultFont(m_FontCreator->CreateFont(
			FontDescription(15, EFontWeight::Bolt), m_FontCreator->GetDefaultCharset("german")));
	}

	SetRenderer(LUX_NEW(Renderer)(video::VideoDriver::Instance()->GetRenderer()));

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader));

	input::InputSystem::Instance()->GetEventSignal().Connect(this, &GUIEnvironment::OnEvent);
	m_Cursor->onCursorMove.Connect(this, &GUIEnvironment::OnCursorMove);

}

GUIEnvironment::~GUIEnvironment()
{
}

StrongRef<Window> GUIEnvironment::GetRootElement()
{
	return m_Root;
}

StrongRef<Cursor> GUIEnvironment::GetCursor()
{
	return m_Cursor;
}

void GUIEnvironment::Render()
{
	m_Root->Render(m_Renderer);
	m_Renderer->Flush();
}

StrongRef<Skin> GUIEnvironment::GetSkin() const
{
	return m_Skin;
}

void GUIEnvironment::SetSkin(Skin* skin)
{
	m_Skin = skin;
}

StrongRef<Renderer> GUIEnvironment::GetRenderer() const
{
	return m_Renderer;
}
void GUIEnvironment::SetRenderer(Renderer* r)
{
	m_Renderer = r;
}

static Element* GetElementByPosRec(Element* e, const math::Vector2F& pos)
{
	Element* out = nullptr;
	if(e->GetFinalRect().IsInside(pos))
		out = e;
	for(auto x : e->Elements())
		out = GetElementByPosRec(x, pos);

	return out;
}

StrongRef<Element> GUIEnvironment::GetElementByPos(const math::Vector2F& pos)
{
	return GetElementByPosRec(m_Root, pos);
}

void GUIEnvironment::SendEvent(Element* elem, const Event& event)
{
	while(elem && !elem->OnEvent(event))
		elem = elem->GetParent();
}

void GUIEnvironment::SendMouseEvent(MouseEvent& e)
{
	e.shift = m_ShiftState;
	e.ctrl = m_ControlState;
	e.leftState = m_LeftState;
	e.rightState = m_RightState;
	e.pos = m_CursorPos;

	if(!m_Hovered)
		m_Hovered = GetElementByPos(e.pos).GetWeak();
	e.elem = m_Hovered;
	SendEvent(e.elem, e);
}

void GUIEnvironment::OnCursorMove(const math::Vector2F& newPos)
{
	if(m_CursorPos != newPos) {
		m_CursorPos = newPos;
		auto newHovered = GetElementByPos(m_CursorPos).GetWeak();
		if(m_Hovered != newHovered) {
			ElementEvent e;
			if(m_Hovered) {
				e.elem = m_Hovered;
				e.type = ElementEvent::MouseLeave;
				SendEvent(e.elem, e);
			}
			if(newHovered) {
				e.elem = newHovered;
				e.type = ElementEvent::MouseEnter;
				SendEvent(e.elem, e);
			}
			m_Hovered = newHovered;
		}

		MouseEvent e;
		e.type = MouseEvent::Move;
		SendMouseEvent(e);
	}
}

void GUIEnvironment::OnEvent(const input::Event& event)
{
	if(event.source == input::EEventSource::Mouse) {
		MouseEvent e;
		if(event.type == input::EEventType::Button) {
			if(event.button.code == input::EKeyCode::KEY_LBUTTON) {
				if(m_LeftState != event.button.state) {
					m_LeftState = event.button.state;
					e.type = event.button.pressedDown ? MouseEvent::LDown : MouseEvent::LUp;
					SendMouseEvent(e);
				}
			}
			if(event.button.code == input::EKeyCode::KEY_RBUTTON) {
				if(m_RightState != event.button.state) {
					m_RightState = event.button.state;
					e.type = event.button.pressedDown ? MouseEvent::RDown : MouseEvent::RUp;
					SendMouseEvent(e);
				}
			}
		}
	}
	if(event.source == input::EEventSource::Keyboard) {
		m_ControlState = event.control;
		m_ShiftState = event.shift;
	}
}

StrongRef<FontCreator> GUIEnvironment::GetFontCreator()
{
	return m_FontCreator;
}

Element* GUIEnvironment::GetHovered()
{
	return m_Hovered;
}

} // namespace gui
} // namespace lux
