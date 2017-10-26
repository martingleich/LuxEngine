#include "gui/elements/GUISlider.h"
#include "gui/GUIEnvironment.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.gui.Slider", lux::gui::Slider);

namespace lux
{
namespace gui
{

Slider::Slider()
{
	m_GrabOffset = 0;
	// Set some "normal" value for the thumb size until the skin is loaded.
	m_ThumbWidth = 10;
	m_ThumbHeight = 30;
	auto minSize = GetMinSize();
	minSize.height = math::Max(GetThumbHeight(), minSize.height);
	SetMinSize(minSize);
	SetSettings(Horizontal | StepOnClick);
	SetRange(0, 100);
	SetStep(1, 10);
	SetThumbPos(0);

	onPosChange.SetCallbackEvent([this](const event::SignalFunc<int>& newSlot) {
		newSlot.Call(m_Pos);
	});
}

Slider::~Slider()
{
}

void Slider::SetSkin(Skin* skin)
{
	Element::SetSkin(skin);
	auto thumbSize = GetSkin()->GetPropertyV("lux.gui.Slider.thumbSize", math::Vector2F(10, 30));
	m_ThumbWidth = thumbSize.x;
	m_ThumbHeight = thumbSize.y;

	auto minSize = GetMinSize();
	minSize.height = math::Max(GetThumbHeight(), minSize.height);
	SetMinSize(minSize);
	SetSettings(GetSettings());
	SetRange(m_MinValue, m_MaxValue);
	SetStep(m_StepSize, m_BigStep);
	SetThumbPos(GetThumbPos());
}

void Slider::Paint(Renderer* renderer)
{
	gui::SliderPaintOptions po;
	po.isHorizontal = IsHorizontal();
	po.palette = GetFinalPalette();
	po.thumbRect = GetThumbRect();
	auto skin = GetSkin();
	skin->DrawControl(renderer,
		this,
		GetFinalRect(),
		gui::EGUIControl::SliderBase,
		GetState(),
		po);
	auto thumbState = GetState();
	if(m_IsPressed)
		thumbState |= gui::EGUIState::Sunken;
	else
		thumbState |= gui::EGUIState::Raised;
	skin->DrawControl(renderer,
		this,
		po.thumbRect,
		gui::EGUIControl::SliderThumb,
		thumbState,
		po);
}

float Slider::GetThumbWidth() const { return m_ThumbWidth; }
float Slider::GetThumbHeight() const { return m_ThumbHeight; }

bool Slider::IsPointOnThumb(const math::Vector2F& point) const
{
	return GetThumbRect().IsInside(point);
}

bool Slider::OnMouseEvent(const gui::MouseEvent& e)
{
	if(e.type == gui::MouseEvent::LDown) {
		if(IsPointInside(e.pos)) {
			bool pressed = false;
			if(m_Settings & ThumbGlideToCursor) {
				SetThumbPos(GetThumbPos(e.pos));
				pressed = true;
				m_GrabOffset = 0;
			} else if((m_Settings & StepOnClick) && !IsPointOnThumb(e.pos)) {
				auto p = GetThumbPos(e.pos);
				if(p < GetThumbPos())
					SetThumbPos(GetThumbPos() - m_StepSize);
				else
					SetThumbPos(GetThumbPos() + m_StepSize);
			} else {
				pressed = true;
				if(m_Settings & CenterThumb)
					SetThumbPos(GetThumbPos(e.pos));
				else
					m_GrabOffset = GetThumbPos() - GetThumbPos(e.pos);
			}

			if(pressed) {
				m_IsPressed = true;
				m_Environment->CaptureCursor(this);
			}
		}
		return true;
	}
	if(e.type == gui::MouseEvent::LUp) {
		m_IsPressed = false;
		m_Environment->ReleaseCursor();
	}
	if(e.type == gui::MouseEvent::Move && m_IsPressed) {
		auto pos = GetThumbPos(e.pos, (m_Settings&CenterThumb) ? 0 : m_GrabOffset);
		SetThumbPos(pos);
	}

	return false;
}

bool Slider::OnKeyboardEvent(const gui::KeyboardEvent& e)
{
	if(e.down) {
		auto step = e.shift ? m_BigStep : m_StepSize;
		step = IsFlipped() ? -step : step;
		if(IsHorizontal()) {
			if(e.key == input::EKeyCode::KEY_LEFT)
				SetThumbPos(GetThumbPos() - step);
			if(e.key == input::EKeyCode::KEY_RIGHT)
				SetThumbPos(GetThumbPos() + step);
		} else {
			if(e.key == input::EKeyCode::KEY_DOWN)
				SetThumbPos(GetThumbPos() - step);
			if(e.key == input::EKeyCode::KEY_UP)
				SetThumbPos(GetThumbPos() + step);
		}
	}
	return false;
}

void Slider::SetThumbPos(int newPos)
{
	if(m_StepSize != 1) {
		int p = newPos - m_MinValue;
		if(p % m_StepSize < m_StepSize / 2)
			p = p / m_StepSize;
		else
			p = p / m_StepSize + 1;
		p *= m_StepSize;
		newPos = p + m_MinValue;
		if(newPos > m_MaxValue)
			newPos -= m_MinValue;
	}
	newPos = math::Clamp(newPos, m_MinValue, m_MaxValue);
	if(newPos != m_Pos) {
		m_Pos = newPos;
		onPosChange.Broadcast(m_Pos);
	}
}
int Slider::GetThumbPos() const
{
	return m_Pos;
}
float Slider::GetRelThumbPos() const
{
	return (float)(m_Pos - m_MinValue) / (m_MaxValue - m_MinValue);
}

void Slider::SetRange(int min, int max)
{
	if(min >= max)
		throw core::InvalidArgumentException("min, max", "Min must be smaller than max");
	m_MinValue = min;
	m_MaxValue = max;
	SetThumbPos(GetThumbPos());
}
void Slider::GetRange(int& min, int& max) const
{
	min = m_MinValue;
	max = m_MaxValue;
}

void Slider::SetStep(int step, int bigStep)
{
	if(step <= 0)
		throw core::InvalidArgumentException("step", "Must be bigger than zero");
	m_StepSize = step;
	m_BigStep = bigStep > 0 ? bigStep : step * 10;
}

int Slider::GetStep() const { return m_StepSize; }
int Slider::GetBigStep() const { return m_BigStep; }

void Slider::AddSettings(int settings)
{
	m_Settings |= settings;
}
void Slider::RemoveSettings(int settings)
{
	m_Settings &= ~settings;
}

int Slider::GetSettings() const { return m_Settings; }
void Slider::SetSettings(int settings) { m_Settings = settings; }
bool Slider::IsFlipped() const { return (m_Settings&Flipped) != 0; }
void Slider::SetFlipped(bool isFlipped)
{
	if(!isFlipped)
		AddSettings(Flipped);
	else
		RemoveSettings(Flipped);
}

bool Slider::IsHorizontal() const { return (m_Settings & Horizontal) != 0; }
void Slider::SetHorizontal(bool isHorizontal)
{
	if(!isHorizontal)
		AddSettings(Horizontal);
	else
		RemoveSettings(Horizontal);
}

core::Name Slider::GetReferableType() const
{
	static const core::Name name = "lux.gui.Slider";
	return name;
}

int Slider::GetThumbPos(const math::Vector2F& curPos, int offset) const
{
	auto rect = GetFinalRect();
	float relPos;
	float relOffset = (float)offset / (m_MaxValue - m_MinValue);
	if(IsHorizontal()) {
		float thumb = GetThumbWidth();
		relPos = math::Clamp(
			(curPos.x - (rect.left + thumb / 2)) / (rect.GetWidth() - thumb) + relOffset, 0.0f, 1.0f);
	} else {
		float thumb = GetThumbHeight();
		relPos = math::Clamp(
			((rect.bottom - thumb / 2) - curPos.y) / (rect.GetHeight() - thumb) + relOffset, 0.0f, 1.0f);
	}
	if(IsFlipped())
		relPos = 1 - relPos;
	return (m_MinValue + std::lround((m_MaxValue - m_MinValue)*relPos));
}

math::RectF Slider::GetThumbRect() const
{
	auto rect = GetFinalRect();
	float thumbWidth = GetThumbWidth();
	float thumbHeight = GetThumbHeight();
	float relThumbPos = GetRelThumbPos();
	if(IsFlipped())
		relThumbPos = 1 - relThumbPos;

	if(IsHorizontal()) {
		float linePos = rect.GetCenter().y;
		float thumbPos = math::Lerp(rect.left + thumbWidth / 2, rect.right - thumbWidth / 2, relThumbPos);
		return math::RectF(
			thumbPos - thumbWidth / 2, linePos - thumbHeight / 2,
			thumbPos + thumbWidth / 2, linePos + thumbHeight / 2);
	} else {
		float linePos = rect.GetCenter().x;
		float thumbPos = math::Lerp(rect.bottom - thumbHeight / 2, rect.top + thumbHeight / 2, relThumbPos);
		return math::RectF(
			linePos - thumbHeight / 2, thumbPos - thumbWidth / 2,
			linePos + thumbHeight / 2, thumbPos + thumbWidth / 2);
	}
}

StrongRef<Referable> Slider::Clone() const
{
	return LUX_NEW(Slider)(*this);
}

} // namespace gui
} // namespace lux
