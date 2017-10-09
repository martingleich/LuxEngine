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

Element::Element() :
	m_Parent(nullptr),
	m_Window(nullptr),
	m_Margin(0, 0, 0, 0),
	m_Size(10, 10),
	m_MinSize(1, 1),
	m_TabId(NO_TAB_STOP),
	m_CanFocus(true),
	m_IsVisible(true),
	m_IsEnabled(true),
	m_NoClip(false)
{
	UpdateRect();
}

Element::~Element()
{
	RemoveAllElements();
}

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
	return (m_Environment->GetFocused() == this);
}

bool Element::IsHovered() const
{
	return (m_Environment->GetHovered() == this);
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

const ScalarDimensionF Element::GetSize() const
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
void Element::SetSize(const ScalarDimensionF& size)
{
	m_Size = size;
	UpdateRect();
}
void Element::SetSize(ScalarDistanceF width, ScalarDistanceF height)
{
	SetSize(ScalarDimensionF(width, height));
}
void Element::SetWidth(ScalarDistanceF width)
{
	SetSize(ScalarDimensionF(width, GetHeight()));
}
void Element::SetHeight(ScalarDistanceF height)
{
	SetSize(ScalarDimensionF(GetWidth(), height));
}

void Element::SetInnerSize(const ScalarDimensionF& size)
{
	auto parent = GetParentInnerRect();
	math::Dimension2F realSize(size.width.GetRealValue(parent.GetWidth()), size.height.GetRealValue(parent.GetHeight()));

	auto& border = GetBorder();
	realSize.width += border.left + border.right;
	realSize.height += border.top + border.bottom;

	SetSize(PixelDimension(realSize.width, realSize.height));
}

const math::Rect<ScalarDistanceF>& Element::GetMargin() const
{
	return m_Margin;
}
void Element::SetMargin(const math::Rect<ScalarDistanceF>& rect)
{
	m_Margin = rect;
	UpdateRect();
}
void Element::SetMargin(ScalarDistanceF left,
	ScalarDistanceF top,
	ScalarDistanceF right,
	ScalarDistanceF bottom)
{
	SetMargin(math::Rect<ScalarDistanceF>(left, top, right, bottom));
}

void Element::SetPlacement(const math::Rect<ScalarDistanceF>& margin, const ScalarDimensionF& size)
{
	m_Margin = margin;
	m_Size = size;
	UpdateRect();
}

void Element::SetMinSize(const math::Dimension2F& minSize)
{
	m_MinSize = minSize;
}

const math::Dimension2F& Element::GetMinSize() const
{
	return m_MinSize;
}

const math::RectF& Element::GetBorder() const
{
	return m_Border;
}

void Element::SetBorder(const math::RectF& border)
{
	m_Border = border;
	UpdateRect();
}

void Element::SetPosition(const ScalarVectorF& pos)
{
	SetPosition(pos.x, pos.y);
}

void Element::SetPosition(ScalarDistanceF x, ScalarDistanceF y)
{
	SetMargin(x, y, AUTO_MARGIN, AUTO_MARGIN);
}

math::Vector2F Element::GetPosition() const
{
	return GetFinalRect().LeftTop() - GetParentInnerRect().LeftTop();
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
	}
	m_Elements.Clear();
}

StrongRef<Element> Element::AddElement(Element* elem)
{
	return AddElement(elem, m_Elements.End());
}

StrongRef<Element> Element::AddElement(Element* elem, ElementIterator before)
{
	StrongRef<Element> elemPtr = elem;
	if(elem->GetParent())
		elem->GetParent()->RemoveElement(elem);
	m_Elements.Insert(elem, before);
	elem->OnAdd(this);
	return elem;
}

const math::RectF& Element::GetFinalInnerRect() const
{
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
	Paint(renderer);

	for(auto e : m_Elements) {
		if(e->IsVisible())
			e->Render(renderer);
	}
}

bool Element::OnEvent(const Event& e)
{
	auto mouseE = dynamic_cast<const gui::MouseEvent*>(&e);
	if(mouseE)
		return OnMouseEvent(*mouseE);
	auto keyE = dynamic_cast<const gui::KeyboardEvent*>(&e);
	if(keyE)
		return OnKeyboardEvent(*keyE);
	auto elementE = dynamic_cast<const gui::ElementEvent*>(&e);
	if(elementE)
		return OnElementEvent(*elementE);

	return true;
}

Skin* Element::GetSkin() const
{
	if(m_Environment)
		return m_Environment->GetSkin();
	else
		return nullptr;
}

void Element::SetFont(Font* f)
{
	m_OverwriteFont = f;
}

StrongRef<Font> Element::GetFont() const
{
	return m_OverwriteFont;
}

StrongRef<Font> Element::GetActiveFont() const
{
	if(m_OverwriteFont)
		return m_OverwriteFont;
	else
		return GetSkin()->defaultFont;
}

EGUIState Element::GetState() const
{
	EGUIState state = EGUIState::Disabled;
	if(IsEnabled()) {
		state |= EGUIState::Enabled;
		if(IsFocused())
			state |= EGUIState::Focused;
		if(IsHovered())
			state |= EGUIState::Highlighted;
	}

	return state;
}

bool Element::IsPointInside(const math::Vector2F& point) const
{
	return GetFinalRect().IsInside(point);
}

void Element::OnAdd(Element* p)
{
	m_Window = p->GetWindow();
	m_Environment = p->GetEnvironment();
	m_Parent = p;

	UpdateRect();

	m_Environment->OnElementAdded(this);
}

void Element::OnRemove(Element* p)
{
	LUX_UNUSED(p);

	m_Environment->OnElementRemoved(this);

	m_Window = nullptr;
	m_Environment = nullptr;
	m_Parent = nullptr;
}

static void CalculateAxis(
	float& finalLow, float& finalHigh,
	float marginLow, float marginHigh,
	float size, float minSize,
	float parentLow, float parentHigh)
{
	bool autoLow = marginLow == Element::AUTO_MARGIN;
	bool autoHigh = marginHigh == Element::AUTO_MARGIN;
	bool autoSize = size == Element::AUTO_SIZE;
	if(!autoSize && size < minSize)
		size = minSize;

	if(autoLow == autoHigh) {
		finalLow = parentLow;
		finalHigh = parentHigh;
		if(!autoLow || !autoHigh) {
			finalLow += marginLow;
			finalHigh -= marginHigh;
		}
		if(autoSize && finalHigh - finalLow < minSize) {
			size = minSize;
			autoSize = false;
		}
		if(!autoSize) {
			const float center = (finalLow + finalHigh) / 2;
			finalLow = center - size / 2;
			finalHigh = center + size / 2;
		}
	} else if(autoHigh) {
		finalLow = parentLow + marginLow;
		if(autoSize) {
			finalHigh = parentHigh;
			if(finalHigh - finalLow < minSize)
				finalHigh = finalLow + minSize;
		} else {
			finalHigh = finalLow + size;
		}
	} else if(autoLow) {
		finalHigh = parentHigh - marginHigh;
		if(autoSize) {
			finalLow = parentLow;
			if(finalHigh - finalLow < minSize) {
				finalLow = finalHigh - minSize;
			}
		} else {
			finalLow = finalHigh - size;
		}
	}
}

void Element::UpdateRect()
{
	if(UpdateFinalRect()) {
		OnFinalRectChange();
	}
}

math::RectF Element::GetParentInnerRect() const
{
	if(!m_Parent)
		return math::RectF(0, 0, 0, 0);

	return m_Parent->GetFinalInnerRect();
}

bool Element::UpdateFinalRect()
{
	auto parentRect = GetParentInnerRect();
	auto parentWidth = parentRect.GetWidth();
	auto parentHeight = parentRect.GetHeight();

	math::RectF margin;
	margin.left = m_Margin.left != AUTO_MARGIN ? m_Margin.left.GetRealValue(parentWidth) : (float)m_Margin.left;
	margin.top = m_Margin.top != AUTO_MARGIN ? m_Margin.top.GetRealValue(parentHeight) : (float)m_Margin.top;
	margin.right = m_Margin.right != AUTO_MARGIN ? m_Margin.right.GetRealValue(parentWidth) : (float)m_Margin.right;
	margin.bottom = m_Margin.bottom != AUTO_MARGIN ? m_Margin.bottom.GetRealValue(parentHeight) : (float)m_Margin.bottom;

	math::Dimension2F size;
	size.width = m_Size.width != AUTO_SIZE ? m_Size.width.GetRealValue(parentWidth) : (float)m_Size.width;
	size.height = m_Size.height != AUTO_SIZE ? m_Size.height.GetRealValue(parentHeight) : (float)m_Size.height;

	math::RectF newRect;
	CalculateAxis(
		newRect.left, newRect.right,
		margin.left, margin.right,
		size.width, m_MinSize.width,
		parentRect.left, parentRect.right);

	CalculateAxis(
		newRect.top, newRect.bottom,
		margin.top, margin.bottom,
		size.height, m_MinSize.height,
		parentRect.top, parentRect.bottom);

	bool changed = (newRect != m_FinalRect);
	m_FinalRect = newRect;
	return changed;
}

bool Element::UpdateInnerRect()
{
	math::RectF newRect;
	newRect.left = m_FinalRect.left + m_Border.left;
	newRect.top = m_FinalRect.top + m_Border.top;
	newRect.right = m_FinalRect.right - m_Border.right;
	newRect.bottom = m_FinalRect.bottom - m_Border.bottom;

	if(!newRect.IsValid()) {
		newRect.right = newRect.left = (newRect.left + newRect.right) / 2;
		newRect.top = newRect.bottom = (newRect.top + newRect.bottom) / 2;
	}

	bool changed = (newRect != m_InnerRect);
	m_InnerRect = newRect;
	return changed;
}

void Element::UpdateChildrenRects() const
{
	for(auto e : m_Elements)
		e->UpdateRect();
}

void Element::OnFinalRectChange()
{
	if(UpdateInnerRect())
		OnInnerRectChange();
}

void Element::OnInnerRectChange()
{
	UpdateChildrenRects();
}

} // namespace gui
} // namespace lux
