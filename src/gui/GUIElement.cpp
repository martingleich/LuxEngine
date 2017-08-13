#include "gui/GUIElement.h"
#include "gui/GUIEnvironment.h"
#include "gui/GUISkin.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace gui
{

const u32 Element::NO_TAB_STOP(0xFFFFFFFF);
const ScalarDistanceF Element::AUTO_SIZE(INFINITY);
const ScalarDistanceF Element::AUTO_MARGIN(INFINITY);

Element* Element::GetParent() const
{
	return m_Parent;
}

Window* Element::GetWindow() const
{
	return m_Window;
}

GUIEnvironment* Element::GetEnvironment() const
{
	return m_Environment;
}

void Element::SetEnvironment(GUIEnvironment* env)
{
	m_Environment = env;
}

void Element::SetTabId(u32 tabId)
{
	m_TabId = tabId;
}

u32 Element::GetTabId() const
{
	return m_TabId;
}
bool Element::IsTabStop() const
{
	return GetTabId() != NO_TAB_STOP;
}

bool Element::IsFocusable() const
{
	return m_CanFocus;
}
void Element::SetFocusable(bool focus)
{
	m_CanFocus = focus;
}

bool Element::IsFocused() const
{
	return false;
}
bool Element::IsTrulyVisible() const
{
	if(!IsVisible())
		return false;
	if(!m_Parent)
		return true;
	return m_Parent->IsTrulyVisible();
}

bool Element::IsVisible() const
{
	return m_IsVisible;
}
void Element::SetVisible(bool visible)
{
	m_IsVisible = visible;
}

bool Element::IsEnabled() const
{
	return m_IsEnabled;
}
void Element::SetEnabled(bool enable)
{
	m_IsEnabled = enable;
}

bool Element::IsClipped() const
{
	return !m_NoClip;
}
void Element::SetClipping(bool clip)
{
	m_NoClip = !clip;
}

const String& Element::GetText() const
{
	return m_Text;
}
void Element::SetText(const String& text)
{
	m_Text = text;
}

const String& Element::GetToolTip() const
{
	return m_ToolTip;
}
void Element::SetToolTip(const String& tip)
{
	m_ToolTip = tip;
}

const math::Dimension2<ScalarDistanceF> Element::GetSize() const
{
	return m_Size;
}
ScalarDistanceF Element::GetWidth() const
{
	return GetSize().width;
}
ScalarDistanceF Element::GetHeight() const
{
	return GetSize().height;
}
void Element::SetSize(const math::Dimension2<ScalarDistanceF>& size)
{
	m_Size = size;
	SetDirtyRect();
}
void Element::SetSize(ScalarDistanceF width, ScalarDistanceF height)
{
	SetSize(math::Dimension2<ScalarDistanceF>(width, height));
}
void Element::SetWidth(ScalarDistanceF width)
{
	SetSize(math::Dimension2<ScalarDistanceF>(width, GetHeight()));
}
void Element::SetHeight(ScalarDistanceF height)
{
	SetSize(math::Dimension2<ScalarDistanceF>(GetWidth(), height));
}

const math::Rect<ScalarDistanceF>& Element::GetMargin() const
{
	return m_Margin;
}
void Element::SetMargin(const math::Rect<ScalarDistanceF>& rect)
{
	m_Margin = rect;
	SetDirtyRect();
}
void Element::SetMargin(ScalarDistanceF left,
	ScalarDistanceF top,
	ScalarDistanceF right,
	ScalarDistanceF bottom)
{
	SetMargin(math::Rect<ScalarDistanceF>(left, top, right, bottom));
}

const math::Rect<ScalarDistanceF>& Element::GetBorder() const
{
	return m_Border;
}

void Element::SetBorder(const math::Rect<ScalarDistanceF>& border)
{
	m_Border = border;
	SetDirtyRect();
}

void Element::SetPosition(const math::Vector2<ScalarDistanceF>& pos)
{
	SetMargin(pos.x, pos.y, AUTO_MARGIN, AUTO_MARGIN);
}

math::Vector2F Element::GetPosition() const
{
	return GetFinalRect().Min() - GetParentInnerRect().Min();
}

core::Range<Element::ElementIterator> Element::Elements()
{
	return core::Range<ElementIterator>(m_Elements.First(), m_Elements.End());
}

core::Range<Element::ConstElementIterator> Element::Elements() const
{
	return core::Range<ConstElementIterator>(m_Elements.First(), m_Elements.End());
}

void Element::RemoveElement(ElementIterator it)
{
	(*it)->OnRemove(this);
	(*it)->Drop();
	m_Elements.Erase(it);
}

void Element::RemoveElement(Element* elem)
{
	auto it = core::LinearSearch(elem, m_Elements);
	if(it != m_Elements.End())
		RemoveElement(it);
}

void Element::RemoveAllElements()
{
	for(auto e : m_Elements) {
		e->OnRemove(this);
		e->Drop();
	}
	m_Elements.Clear();
}

StrongRef<Element> Element::AddElement(Element* elem)
{
	return AddElement(elem, m_Elements.End());
}

StrongRef<Element> Element::AddElement(Element* elem, ElementIterator before)
{
	elem->Grab();
	m_Elements.Insert(elem, before);
	elem->OnAdd(this);
	return elem;
}

const math::RectF& Element::GetFinalInnerRect() const
{
	if(m_DirtyRect)
		UpdateFinalRect();
	return m_InnerRect;
}

math::Dimension2F Element::GetFinalInnerSize() const
{
	return GetFinalInnerRect().GetSize();
}

float Element::GetFinalInnerWidth() const
{
	return GetFinalInnerSize().width;
}

float Element::GetFinalInnerHeight() const
{
	return GetFinalInnerSize().height;
}

const math::RectF& Element::GetFinalRect() const
{
	if(m_DirtyRect)
		UpdateFinalRect();
	return m_FinalRect;
}

math::Dimension2F Element::GetFinalSize() const
{
	return GetFinalRect().GetSize();
}

float Element::GetFinalWidth() const
{
	return GetFinalSize().width;
}

float Element::GetFinalHeight() const
{
	return GetFinalSize().height;
}

void Element::Render(Renderer* renderer)
{
	for(auto e : m_Elements) {
		if(e->IsVisible())
			e->Render(renderer);
	}
}

bool Element::OnEvent(const Event& e)
{
	LUX_UNUSED(e);
	return true;
}

Skin* Element::GetSkin() const
{
	if(m_Environment)
		return m_Environment->GetSkin();
	else
		return nullptr;
}

void Element::OnAdd(Element* p)
{
	m_Window = p->GetWindow();
	m_Environment = p->GetEnvironment();
	m_Parent = p;
	SetDirtyRect();
}

void Element::OnRemove(Element* p)
{
	LUX_UNUSED(p);
	m_Window = nullptr;
	m_Environment = nullptr;
	m_Parent = nullptr;
	SetDirtyRect();
}

static void CalculateAxis(
	float& finalLow, float& finalHigh,
	float marginLow, float marginHigh,
	float size,
	float parentLow, float parentHigh)
{
	const bool autoLow = marginLow == Element::AUTO_MARGIN;
	const bool autoHigh = marginHigh == Element::AUTO_MARGIN;
	const bool autoSize = size == Element::AUTO_SIZE;

	if(autoLow == autoHigh) {
		finalLow = parentLow;
		finalHigh = parentHigh;
		if(!autoLow || !autoHigh) {
			finalLow += marginLow;
			finalHigh -= marginHigh;
		}

		if(!autoSize) {
			const float center = (finalLow + finalHigh) / 2;
			finalLow = center - size / 2;
			finalHigh = center + size / 2;
		}
	} else if(autoHigh) {
		finalLow = parentLow + marginLow;
		if(autoSize)
			finalHigh = parentHigh;
		else
			finalHigh = finalLow + size;
	} else if(autoLow) {
		finalHigh = parentHigh - marginHigh;
		if(autoSize)
			finalLow = parentLow;
		else
			finalLow = finalHigh - size;
	}
}

void Element::UpdateRect() const
{
	if(!m_DirtyRect)
		return;
	m_DirtyRect = false;

	UpdateFinalRect();
	UpdateInnerRect();
}

math::RectF Element::GetParentInnerRect() const
{
	if(!m_Parent)
		return math::RectF(0, 0, 0, 0);

	return m_Parent->GetFinalInnerRect();
}

void Element::UpdateFinalRect() const
{
	auto parentRect = GetParentInnerRect();
	auto parentWidth = parentRect.GetWidth();
	auto parentHeight = parentRect.GetHeight();

	math::RectF margin;
	margin.left = m_Margin.left != AUTO_MARGIN ? m_Margin.left.GetRealValue(parentWidth) : m_Margin.left;
	margin.top = m_Margin.top != AUTO_MARGIN ? m_Margin.top.GetRealValue(parentHeight) : m_Margin.top;
	margin.right = m_Margin.right != AUTO_MARGIN ? m_Margin.right.GetRealValue(parentWidth) : m_Margin.right;
	margin.bottom = m_Margin.bottom != AUTO_MARGIN ? m_Margin.bottom.GetRealValue(parentHeight) : m_Margin.bottom;

	math::Dimension2F size;
	size.width = m_Size.width != AUTO_SIZE ? m_Size.width.GetRealValue(parentWidth) : m_Size.width;
	size.height = m_Size.height != AUTO_SIZE ? m_Size.height.GetRealValue(parentHeight) : m_Size.height;

	CalculateAxis(
		m_FinalRect.left, m_FinalRect.right,
		margin.left, margin.right,
		size.width,
		parentRect.left, parentRect.right);

	CalculateAxis(
		m_FinalRect.top, m_FinalRect.bottom,
		margin.top, margin.bottom,
		size.height,
		parentRect.top, parentRect.bottom);
}

void Element::UpdateInnerRect() const
{
	m_InnerRect.left = m_FinalRect.left + m_Border.left;
	m_InnerRect.top = m_FinalRect.top + m_Border.top;
	m_InnerRect.right = m_FinalRect.right - m_Border.right;
	m_InnerRect.bottom = m_FinalRect.bottom - m_Border.bottom;

	if(!m_InnerRect.IsValid()) {
		m_InnerRect.right = m_InnerRect.left = (m_InnerRect.left + m_InnerRect.right) / 2;
		m_InnerRect.top = m_InnerRect.bottom = (m_InnerRect.top + m_InnerRect.bottom) / 2;
	}
}

void Element::SetDirtyRect()
{
	m_DirtyRect = true;
	for(auto e : m_Elements)
		e->SetDirtyRect();
}

} // namespace gui
} // namespace lux