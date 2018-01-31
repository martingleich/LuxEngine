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
#include "gui/elements/GUISlider.h"
#include "gui/elements/GUICheckBox.h"
#include "gui/elements/GUIRadioButton.h"
#include "gui/elements/GUIImageDisplay.h"
#include "gui/elements/GUITextBox.h"

#include "gui/FontFormat.h"
#include "gui/FontCreator.h"

#include "gui/GUIRenderer.h"

#include "gui/CursorLux.h"

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

class GUIEnvironment::CursorControl : public Cursor
{
public:
	CursorControl(Cursor*& cursor) :
		m_Cursor(cursor)
	{
		m_Visible = cursor->IsVisible();
		m_VirtualVisible = false;
	}

	void SetState(ECursorState state) { m_Cursor->SetState(state); }
	ECursorState GetState() const { return m_Cursor->GetState(); }
	void SetPosition(float x, float y) { m_Cursor->SetPosition(x, y); }
	void SetRelPosition(float x, float y) { return m_Cursor->SetRelPosition(x, y); }
	math::Vector2F GetPosition() const { return m_Cursor->GetPosition(); }
	math::Vector2F GetRelPosition() const { return m_Cursor->GetRelPosition(); }
	const math::Dimension2F& GetScreenSize() const { return m_Cursor->GetScreenSize(); }
	void SetVisible(bool visible)
	{
		m_Visible = visible;
		if(!m_VirtualVisible)
			m_Cursor->SetVisible(visible);
	}

	bool IsVisible() const
	{
		return m_Visible;
	}

	bool IsGrabbing() const { return m_Cursor->IsGrabbing(); }
	void SetGrabbing(bool grab) { m_Cursor->SetGrabbing(grab); }
	const math::Vector2F& GetGrabbingPosition() const { return m_Cursor->GetGrabbingPosition(); }

	void SetVirtualDraw(bool b)
	{
		m_VirtualVisible = b;
		if(b)
			m_Visible = m_Cursor->IsVisible();
	}

private:
	Cursor*& m_Cursor;
	bool m_Visible;
	bool m_VirtualVisible;
};

GUIEnvironment::GUIEnvironment(Window* osWindow, Cursor* osCursor) :
	m_OSCursor(osCursor),
	m_OSWindow(osWindow),
	m_Cursor(nullptr),
	m_LeftState(false),
	m_RightState(false),
	m_ControlState(false),
	m_ShiftState(false),
	m_IgnoreMouse(false),
	m_IgnoreKeyboard(false),
	m_UseVirtualCursor(true), // Is later in code set to false
	m_DrawVirtualCursor(false)
{
	m_KeyRepeatContext.isActive = false;
	m_KeyRepeatContext.keyRepeatTime = 0.03f;
	m_KeyRepeatContext.keyRepeatStartTime = 0.25f;

	m_OSWindow->SetEnvironment(this);

	m_Root = m_OSWindow;
	m_LuxCursor = LUX_NEW(CursorLux)(m_Root);
	UseVirtualCursor(false);

	m_CursorCtrl = LUX_NEW(CursorControl)(m_Cursor);
	m_CursorCtrl->SetVirtualDraw(false);

#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32);
#else
	m_FontCreator = nullptr;
#endif

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader));
	core::ResourceSystem::Instance()->AddResourceWriter(LUX_NEW(FontWriter));

	/*
	auto m_BuiltInFont = GetFontCreator()->CreateFont(
		FontDescription("Arial", 16, EFontWeight::Bolt),
		GetFontCreator()->GetDefaultCharset("german"));
	*/
	try {
		auto file = io::FileSystem::Instance()->OpenVirtualFile(
			g_BuiltinFontData, g_BuiltinFontData_SIZE, "[builtin_font_file]", false);
		m_BuiltInFont = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Font, file).As<Font>();
	} catch(...) {
		log::Error("Can't load builtin font files");
	}

	SetSkin(core::ReferableFactory::Instance()->Create(core::Name("lux.gui.skin.3D")).As<gui::Skin>());
	m_Skin->SetDefaultFont(m_BuiltInFont);

	SetRenderer(LUX_NEW(Renderer)(video::VideoDriver::Instance()->GetRenderer()));

	input::InputSystem::Instance()->GetEventSignal().Connect(this, &GUIEnvironment::OnEvent);
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
	return m_CursorCtrl;
}

void GUIEnvironment::UseVirtualCursor(bool useVirtual)
{
	if(useVirtual == m_UseVirtualCursor)
		return;

	m_UseVirtualCursor = useVirtual;

	if(m_Cursor)
		m_Cursor->onCursorMove.DisconnectClass(this);
	if(useVirtual) {
		if(m_Cursor) {
			m_LuxCursor->SetVisible(m_OSCursor->IsVisible());
			m_LuxCursor->SetGrabbing(m_OSCursor->IsGrabbing());
			m_LuxCursor->SetPosition(m_OSCursor->GetPosition());
			if(!m_Root->GetFinalInnerRect().IsInside(m_LuxCursor->GetPosition())) {
				m_LuxCursor->SetRelPosition(0.5f, 0.5f);
			}
		} else {
			m_LuxCursor->SetVisible(true);
			m_LuxCursor->SetGrabbing(false);
		}
		m_OSCursor->SetGrabbing(true);
		m_OSCursor->SetVisible(false);
		m_Cursor = m_LuxCursor;
	} else {
		if(m_Cursor) {
			m_OSCursor->SetVisible(m_LuxCursor->IsVisible());
			m_OSCursor->SetGrabbing(m_LuxCursor->IsGrabbing());
			m_OSCursor->SetPosition(m_LuxCursor->GetPosition());
		}
		m_LuxCursor->SetGrabbing(true);
		m_LuxCursor->SetVisible(false);
		m_Cursor = m_OSCursor;
	}

	m_Cursor->onCursorMove.Connect(this, &GUIEnvironment::OnCursorMove);
	OnCursorMove(m_Cursor->GetPosition());
}

void GUIEnvironment::SetDrawVirtualCursor(bool draw)
{
	if(m_DrawVirtualCursor == draw)
		return;
	m_DrawVirtualCursor = draw;

	if(draw)
		m_CursorCtrl->SetVirtualDraw(draw);
	if(draw)
		m_OSCursor->SetVisible(false);
	else
		m_OSCursor->SetVisible(m_CursorCtrl->IsVisible());
	if(!draw)
		m_CursorCtrl->SetVirtualDraw(draw);
}

///////////////////////////////////////////////////////////////////////////

void GUIEnvironment::Update(float secsPassed)
{
	m_SecsPassed = secsPassed;

	auto newHovered = GetElementByPos(m_CursorPos).GetWeak();
	if(m_Hovered != newHovered) {
		ElementEvent e;
		if(m_Hovered) {
			e.elem = m_Hovered;
			e.type = ElementEvent::MouseLeave;
			SendElementEvent(e.elem, e);
		}
		m_Hovered = newHovered;
		if(newHovered) {
			e.elem = newHovered;
			e.type = ElementEvent::MouseEnter;
			SendElementEvent(e.elem, e);
			auto newCur = newHovered->GetHoverCursor();
			if(newCur != ECursorState::Default) {
				if(m_CursorCtrl->GetState() != newCur)
					m_CursorCtrl->SetState(newCur);
			} else {
				if(m_CursorCtrl->GetState() != newCur)
					m_CursorCtrl->SetState(ECursorState::Normal);
			}
		}
	}

	if(m_KeyRepeatContext.isActive) {
		if(m_KeyRepeatContext.timeToStart <= 0) {
			if(m_KeyRepeatContext.timeToRepeat <= 0) {
				SendKeyboardEvent(m_KeyRepeatContext.event);
				m_KeyRepeatContext.timeToRepeat = m_KeyRepeatContext.keyRepeatTime;
			} else {
				m_KeyRepeatContext.timeToRepeat -= secsPassed;
			}
		} else {
			m_KeyRepeatContext.timeToStart -= secsPassed;
		}
	}
}

static void RecursiveRender(gui::Element* elem, gui::Renderer* renderer, float secsPassed)
{
	elem->Paint(renderer, secsPassed);
	for(auto& e : elem->Elements()) {
		if(e->IsVisible())
			RecursiveRender(e, renderer, secsPassed);
	}
}

void GUIEnvironment::Render()
{
	m_Time += m_SecsPassed;

	video::RenderStatistics::GroupScope grpScope("gui");

	m_Renderer->Begin();
	RecursiveRender(m_Root, m_Renderer, m_SecsPassed);

	if(m_Focused)
		m_Skin->DrawFocus(m_Renderer, m_Focused);

	if((m_DrawVirtualCursor || m_UseVirtualCursor) && m_CursorCtrl->IsVisible() &&
		m_Root->GetFinalInnerRect().IsInside(m_CursorCtrl->GetPosition()))
		m_Skin->DrawCursor(m_Renderer, m_CursorCtrl->GetState(), m_LeftState, m_CursorCtrl->GetPosition(), m_Time);

	m_Renderer->Flush();

	m_SecsPassed = 0;
}

///////////////////////////////////////////////////////////////////////////

StrongRef<Skin> GUIEnvironment::GetSkin() const
{
	return m_Skin;
}

void GUIEnvironment::SetSkin(Skin* skin)
{
	m_Skin = skin;
	m_Root->SetOverwriteSkin(m_Skin);
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

void GUIEnvironment::SetKeyRepeat(float timeToStart, float timeToRepeat)
{
	if(timeToStart < 0)
		throw core::InvalidArgumentException("timeToStart", "Must be non-negative");
	if(timeToRepeat < 0)
		throw core::InvalidArgumentException("timeToRepeat", "Must be non-negative");

	m_KeyRepeatContext.keyRepeatStartTime = timeToStart;
	m_KeyRepeatContext.keyRepeatTime = timeToRepeat;
}

void GUIEnvironment::GetKeyRepeat(float& timeToStart, float& timeToRepeat)
{
	timeToStart = m_KeyRepeatContext.keyRepeatStartTime;
	timeToRepeat = m_KeyRepeatContext.keyRepeatStartTime;
}

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

		if(m_Cursor == m_LuxCursor) {
			if(event.type == input::EEventType::Area)
				m_LuxCursor.As<CursorLux>()->Move(math::Vector2F(event.area.relX, event.area.relY));
		}
	}
	if(event.source == input::EEventSource::Keyboard) {
		m_ControlState = event.control;
		m_ShiftState = event.shift;
		KeyboardEvent e;
		e.autoRepeat = false;
		memcpy(e.character, event.keyInput.character, sizeof(e.character));
		e.key = event.button.code;
		e.down = event.button.state;
		SendKeyboardEvent(e);

		if(e.down) {
			m_KeyRepeatContext.event = e;
			m_KeyRepeatContext.event.autoRepeat = true;
			m_KeyRepeatContext.isActive = true;
			m_KeyRepeatContext.timeToStart = m_KeyRepeatContext.keyRepeatStartTime;
			m_KeyRepeatContext.timeToRepeat = 0;
		} else {
			m_KeyRepeatContext.isActive = false;
		}
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

bool GUIEnvironment::SendElementEvent(Element* elem, const Event& event)
{
	return elem->OnEvent(event);
}

Element* GUIEnvironment::GetHovered()
{
	return m_Hovered;
}

void GUIEnvironment::SetFocused(Element* elem)
{
	if(!elem)
		m_Focused = nullptr;
	if(m_Focused != elem && elem->IsFocusable() && elem->IsEnabled()) {
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

StrongRef<StaticText> GUIEnvironment::AddStaticText(const ScalarVectorF& position, const core::String& text)
{
	StrongRef<StaticText> st = AddElement(core::Name("lux.gui.StaticText"));
	st->SetPosition(position);
	st->SetText(text);
	return st;
}

StrongRef<Button> GUIEnvironment::AddButton(const ScalarVectorF& pos, const ScalarDimensionF& size, const core::String& text)
{
	StrongRef<Button> button = AddElement(core::Name("lux.gui.Button"));
	button->SetPosition(pos);
	button->SetSize(size);
	button->SetText(text);
	return button;
}

StrongRef<Slider> GUIEnvironment::AddSlider(const ScalarVectorF& pos, const ScalarDistanceF& size, int min, int max)
{
	StrongRef<Slider> slider = AddElement(core::Name("lux.gui.Slider"));
	slider->SetPosition(pos);
	slider->SetWidth(size);
	slider->SetRange(min, max);
	slider->SetThumbPos(min);
	return slider;
}

StrongRef<Slider> GUIEnvironment::AddVerticalSlider(const ScalarVectorF& pos, const ScalarDistanceF& size, int min, int max)
{
	StrongRef<Slider> slider = AddElement(core::Name("lux.gui.Slider"));
	slider->SetHorizontal(false);
	slider->SetPosition(pos);
	slider->SetHeight(size);
	slider->SetRange(min, max);
	slider->SetThumbPos(min);
	return slider;
}

StrongRef<CheckBox> GUIEnvironment::AddCheckBox(const ScalarVectorF& pos, const ScalarDimensionF& size, bool checked)
{
	StrongRef<CheckBox> box = AddElement(core::Name("lux.gui.CheckBox"));
	box->SetPosition(pos);
	box->SetSize(size);
	box->SetChecked(checked);
	return box;
}

StrongRef<RadioButton> GUIEnvironment::AddRadioButton(const ScalarVectorF& pos, const ScalarDimensionF& size, RadioButton* group)
{
	StrongRef<RadioButton> radio = AddElement(core::Name("lux.gui.RadioButton"));
	radio->SetPosition(pos);
	radio->SetSize(size);
	if(group)
		radio->SetRadioGroup(group);
	return radio;
}

StrongRef<ImageDisplay> GUIEnvironment::AddImageDisplay(const ScalarVectorF& pos, const ScalarDimensionF& size, video::Texture* img)
{
	StrongRef<ImageDisplay> imgDisplay = AddElement(core::Name("lux.gui.ImageDisplay"));
	imgDisplay->SetPosition(pos);
	imgDisplay->SetSize(size);
	imgDisplay->SetTexture(img);

	return imgDisplay;
}

StrongRef<TextBox> GUIEnvironment::AddTextBox(const ScalarVectorF& pos, const ScalarDimensionF& size)
{
	StrongRef<TextBox> textBox = AddElement(core::Name("lux.gui.TextBox"));
	textBox->SetPosition(pos);
	textBox->SetSize(size);

	return textBox;
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

void GUIEnvironment::CaptureCursor(Element* elem)
{
	m_Captured = elem;
}

void GUIEnvironment::ReleaseCursor()
{
	m_Captured = nullptr;
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

void GUIEnvironment::OnElementAdded(Element* elem)
{
	LUX_UNUSED(elem);
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
	e.elem = nullptr;

	if(m_Captured) {
		e.elem = m_Captured;
		if(e.elem->IsEnabled())
			SendElementEvent(e.elem, e);
	} else {
		if(m_Hovered) {
			e.elem = m_Hovered;
			if(e.elem->IsEnabled())
				SendElementEvent(e.elem, e);
		}
	}
	if(e.elem && e.IsClick()) {
		if(e.elem->IsFocusable())
			SetFocused(e.elem);
		else
			SetFocused(nullptr);
	}
}

void GUIEnvironment::SendKeyboardEvent(KeyboardEvent& e)
{
	e.shift = m_ShiftState;
	e.ctrl = m_ControlState;

	if(m_Focused) {
		e.elem = m_Focused;
		if(e.elem->IsEnabled())
			SendElementEvent(e.elem, e);
	}
}

} // namespace gui
} // namespace lux
