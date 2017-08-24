#include "gui/GUIEnvironment.h"
#include "core/Logger.h"

#include "input/InputSystem.h"

#include "video/VideoDriver.h"

#include "core/ReferableFactory.h"

#include "gui/Cursor.h"
#include "gui/Window.h"
#include "gui/GUISkin.h"
#include "gui/GUIElement.h"

#include "gui/elements/GUIButton.h"
#include "gui/elements/GUIStaticText.h"

#include "gui/FontFormat.h"
#include "gui/FontCreator.h"

#include "io/FileSystem.h"
#include "io/File.h"

#ifdef LUX_WINDOWS
#include "gui/FontCreatorWin32.h"
#endif

#include "gui/BuiltInFont.h"

namespace lux
{
namespace gui
{

GUIEnvironment::GUIEnvironment(Window* osWindow, Cursor* osCursor) :
	m_OSWindow(osWindow),
	m_OSCursor(osCursor),
	m_IgnoreMouse(false),
	m_IgnoreKeyboard(false)
{
	m_OSWindow->SetEnvironment(this);

	m_Root = m_OSWindow;
	m_Cursor = m_OSCursor;


#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32);
#else
	m_FontCreator = nullptr;
#endif

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader));
	core::ResourceSystem::Instance()->AddResourceWriter(LUX_NEW(FontWriter));

	auto file = io::FileSystem::Instance()->OpenVirtualFile(
		g_BuiltinFontData, g_BuiltinFontData_Size, "[builtin_font_file]", false);
	if(file)
		m_BuiltInFont = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Font, file).As<Font>();

	SetSkin(LUX_NEW(Skin3D));
	m_Skin->defaultFont = m_BuiltInFont;

	SetRenderer(LUX_NEW(Renderer)(video::VideoDriver::Instance()->GetRenderer()));

	input::InputSystem::Instance()->GetEventSignal().Connect(this, &GUIEnvironment::OnEvent);
	m_Cursor->onCursorMove.Connect(this, &GUIEnvironment::OnCursorMove);
}

GUIEnvironment::~GUIEnvironment()
{
}

///////////////////////////////////////////////////////////////////////////

StrongRef<Window> GUIEnvironment::GetRootElement()
{
	return m_Root;
}

StrongRef<Cursor> GUIEnvironment::GetCursor()
{
	return m_Cursor;
}

///////////////////////////////////////////////////////////////////////////

void GUIEnvironment::Update(float secsPassed)
{
	LUX_UNUSED(secsPassed);

	auto newHovered = GetElementByPos(m_CursorPos).GetWeak();
	if(m_Hovered != newHovered) {
		ElementEvent e;
		if(m_Hovered) {
			e.elem = m_Hovered;
			e.type = ElementEvent::MouseLeave;
			SendElementEvent(e.elem, e);
		}
		if(newHovered) {
			e.elem = newHovered;
			e.type = ElementEvent::MouseEnter;
			SendElementEvent(e.elem, e);
		}
		//onHoverChange.Broadcast(newHovered);
		m_Hovered = newHovered;
	}
}

void GUIEnvironment::Render()
{
	video::RenderStatistics::GroupScope grpScope("gui");
	m_Renderer->Begin();
	m_Root->Render(m_Renderer);
	m_Renderer->Flush();
}

///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////

void GUIEnvironment::IgnoreKeyboard(bool b)
{
	m_IgnoreKeyboard = b;
}

void GUIEnvironment::IgnoreMouse(bool b)
{
	m_IgnoreMouse = b;
}

void GUIEnvironment::SendUserInputEvent(const input::Event& event)
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
		KeyboardEvent e;
		e.autoRepeat = !event.button.pressedDown;
		memcpy(e.character, event.keyInput.character, sizeof(e.character));
		e.key = event.button.code;
		e.down = event.button.state;
		SendKeyboardEvent(e);
	}
}

static Element* GetElementByPosRec(Element* e, const math::Vector2F& pos)
{
	Element* out = nullptr;
	if(e->IsPointInside(pos))
		out = e;
	for(auto x : e->Elements()) {
		auto childOut = GetElementByPosRec(x, pos);
		if(childOut)
			return childOut;
	}

	return out;
}

StrongRef<Element> GUIEnvironment::GetElementByPos(const math::Vector2F& pos)
{
	return GetElementByPosRec(m_Root, pos);
}

void GUIEnvironment::SendElementEvent(Element* elem, const Event& event)
{
	elem->OnEvent(event);
}

Element* GUIEnvironment::GetHovered()
{
	return m_Hovered;
}

void GUIEnvironment::SetFocused(Element* elem)
{
	if(!elem)
		m_Focused = nullptr;
	if(m_Focused != elem && elem->IsFocusable()) {
		m_Focused = elem;
		ElementEvent e;
		e.elem = elem;
		e.type = ElementEvent::FocusGained;
		SendElementEvent(e.elem, e);
	}
}

Element* GUIEnvironment::GetFocused()
{
	return m_Focused;
}

///////////////////////////////////////////////////////////////////////////

StrongRef<Element> GUIEnvironment::AddElement(core::Name name, Element* parent)
{
	if(!parent)
		parent = m_Root;

	return parent->AddElement(core::ReferableFactory::Instance()->Create(name).As<Element>());
}

StrongRef<StaticText> GUIEnvironment::AddStaticText(const ScalarVectorF& position, const String& text, Element* parent)
{
	StrongRef<StaticText> st = AddElement("lux.gui.StaticText", parent);
	st->SetPosition(position);
	st->SetText(text);
	return st;
}

StrongRef<Button> GUIEnvironment::AddButton(const ScalarVectorF& pos, const ScalarDimensionF& size, const String& text, Element* parent)
{
	StrongRef<Button> button = AddElement("lux.gui.Button", parent);
	button->SetPosition(pos);
	button->SetSize(size);
	button->SetText(text);
	return button;
}

StrongRef<Button> GUIEnvironment::AddSwitchButton(const ScalarVectorF& pos, const ScalarDimensionF& size, const String& text, Element* parent)
{
	StrongRef<Button> button = AddElement("lux.gui.SwitchButton", parent);
	button->SetPosition(pos);
	button->SetSize(size);
	button->SetText(text);
	return button;
}

///////////////////////////////////////////////////////////////////////////

StrongRef<FontCreator> GUIEnvironment::GetFontCreator()
{
	return m_FontCreator;
}

StrongRef<Font> GUIEnvironment::GetBuiltInFont()
{
	return m_BuiltInFont;
}

///////////////////////////////////////////////////////////////////////////

WeakRef<Element> GUIEnvironment::GrabCursor(Element* elem)
{
	auto out = m_Grabbed;
	m_Grabbed = elem;
	return out;
}

void GUIEnvironment::OnElementRemoved(Element* elem)
{
	if(elem == GetHovered()) {
		ElementEvent e;
		e.elem = elem;
		e.type = ElementEvent::MouseLeave;
		SendElementEvent(e.elem, e);
	}
	if(elem == GetFocused()) {
		ElementEvent e;
		e.elem = elem;
		e.type = ElementEvent::FocusLost;
		SendElementEvent(e.elem, e);
		m_Focused = nullptr;
	}

	for(auto e : elem->Elements())
		OnElementRemoved(e);
}

///////////////////////////////////////////////////////////////////////////

void GUIEnvironment::OnCursorMove(const math::Vector2F& newPos)
{
	if(m_CursorPos != newPos) {
		m_CursorPos = newPos;
		MouseEvent e;
		e.type = MouseEvent::Move;
		SendMouseEvent(e);
	}
}

void GUIEnvironment::OnEvent(const input::Event& event)
{
	if((event.source == input::EEventSource::Mouse && !m_IgnoreMouse) ||
		(event.source == input::EEventSource::Keyboard && !m_IgnoreKeyboard))
		SendUserInputEvent(event);
}

void GUIEnvironment::SendMouseEvent(MouseEvent& e)
{
	e.shift = m_ShiftState;
	e.ctrl = m_ControlState;
	e.leftState = m_LeftState;
	e.rightState = m_RightState;
	e.pos = m_CursorPos;

	if(m_Grabbed) {
		e.elem = m_Grabbed;
		SendElementEvent(e.elem, e);
	} else {
		if(m_Hovered) {
			e.elem = m_Hovered;
			SendElementEvent(e.elem, e);
		}
	}
}

void GUIEnvironment::SendKeyboardEvent(KeyboardEvent& e)
{
	e.shift = m_ShiftState;
	e.ctrl = m_ControlState;

	if(m_Focused) {
		e.elem = m_Focused;
		SendElementEvent(e.elem, e);
	}
}

} // namespace gui
} // namespace lux
