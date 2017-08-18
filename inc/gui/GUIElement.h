#ifndef INCLUDED_GUI_ELEMENT_H
#define INCLUDED_GUI_ELEMENT_H
#include "core/Referable.h"
#include "math/Rect.h"
#include "core/lxArray.h"
#include "events/lxSignal.h"

#include "gui/GUISkin.h"
#include "gui/GUIScalarDistance.h"

namespace lux
{
namespace gui
{

class Event;
class MouseEvent;
class ElementEvent;
class Window;
class Renderer;
class Skin;
class GUIEnvironment;

class Element : public Referable
{
public:
	using ElementList = core::Array<Element*>;
	using ElementIterator = ElementList::Iterator;
	using ConstElementIterator = ElementList::ConstIterator;

public:
	LUX_API static const u32 NO_TAB_STOP;
	LUX_API static const ScalarDistanceF AUTO_SIZE;
	LUX_API static const ScalarDistanceF AUTO_MARGIN;

public:
	LUX_API Element();
	LUX_API virtual ~Element();

	LUX_API Element* GetParent() const;

	LUX_API Window* GetWindow() const;
	LUX_API GUIEnvironment* GetEnvironment() const;
	LUX_API void SetEnvironment(GUIEnvironment* env);

	LUX_API virtual void SetTabId(u32 tabId);
	LUX_API virtual u32 GetTabId() const;
	LUX_API virtual bool IsTabStop() const;

	LUX_API virtual bool IsFocusable() const;
	LUX_API virtual void SetFocusable(bool focus);
	LUX_API virtual bool IsFocused() const;

	LUX_API virtual bool IsTrulyVisible() const;
	LUX_API virtual bool IsVisible() const;
	LUX_API virtual void SetVisible(bool visible);

	LUX_API virtual bool IsEnabled() const;
	LUX_API virtual void SetEnabled(bool enable);

	LUX_API virtual bool IsClipped() const;
	LUX_API virtual void SetClipping(bool clip);

	LUX_API virtual const String& GetText() const;
	LUX_API virtual void SetText(const String& text);

	LUX_API virtual const String& GetToolTip() const;
	LUX_API virtual void SetToolTip(const String& tip);

	LUX_API virtual void SetBorder(const math::Rect<ScalarDistanceF>& border);
	LUX_API virtual const math::Rect<ScalarDistanceF>& GetBorder() const;

	LUX_API virtual const ScalarDimensionF GetSize() const;
	LUX_API ScalarDistanceF GetWidth() const;
	LUX_API ScalarDistanceF GetHeight() const;
	LUX_API virtual void SetSize(const ScalarDimensionF& size);
	LUX_API void SetSize(ScalarDistanceF width, ScalarDistanceF heigth);
	LUX_API void SetWidth(ScalarDistanceF width);
	LUX_API void SetHeight(ScalarDistanceF width);

	LUX_API virtual const math::Rect<ScalarDistanceF>& GetMargin() const;
	LUX_API virtual void SetMargin(const math::Rect<ScalarDistanceF>& rect);
	LUX_API void SetMargin(ScalarDistanceF left, ScalarDistanceF top,
		ScalarDistanceF right, ScalarDistanceF bottom);

	LUX_API virtual void SetPlacement(const math::Rect<ScalarDistanceF>& margin, const ScalarDimensionF& size);

	LUX_API void SetPosition(const ScalarVectorF & pos);
	LUX_API void SetPosition(ScalarDistanceF x, ScalarDistanceF y);
	LUX_API math::Vector2F GetPosition() const;

	LUX_API virtual core::Range<ElementIterator> Elements();
	LUX_API virtual core::Range<ConstElementIterator> Elements() const;

	LUX_API virtual void RemoveElement(ElementIterator it);
	LUX_API void RemoveElement(Element* elem);
	LUX_API virtual void RemoveAllElements();

	LUX_API StrongRef<Element> AddElement(Element* elem);
	LUX_API virtual StrongRef<Element> AddElement(Element* elem, ElementIterator where);

	LUX_API virtual const math::RectF& GetFinalInnerRect() const;
	LUX_API math::Dimension2F GetFinalInnerSize() const;
	LUX_API float GetFinalInnerWidth() const;
	LUX_API float GetFinalInnerHeight() const;

	LUX_API virtual const math::RectF& GetFinalRect() const;
	LUX_API math::Dimension2F GetFinalSize() const;
	LUX_API float GetFinalWidth() const;
	LUX_API float GetFinalHeight() const;

	LUX_API virtual bool OnEvent(const Event& e);
	virtual bool OnMouseEvent(const gui::MouseEvent& e) { LUX_UNUSED(e); return false; }
	virtual bool OnElementEvent(const gui::ElementEvent& e) { LUX_UNUSED(e); return false; }

	LUX_API virtual void Render(Renderer* renderer);
	virtual void Paint(Renderer* renderer) { LUX_UNUSED(renderer); }

	LUX_API virtual Skin* GetSkin() const;
	LUX_API EGUIState GetState() const;

protected:
	LUX_API virtual void OnAdd(Element* p);
	LUX_API virtual void OnRemove(Element* p);

	LUX_API virtual void UpdateRect();

	LUX_API virtual math::RectF GetParentInnerRect() const;
	LUX_API virtual bool UpdateFinalRect();
	LUX_API virtual bool UpdateInnerRect();

	LUX_API void UpdateChildrenRects() const;

	LUX_API void OnFinalRectChange();
	LUX_API void OnInnerRectChange();

protected:
	ElementList m_Elements;

	GUIEnvironment* m_Environment;
	Element* m_Parent;
	Window* m_Window;

	String m_Text;
	String m_ToolTip;

	mutable math::RectF m_FinalRect;
	mutable math::RectF m_InnerRect;

	math::Rect<ScalarDistanceF> m_Border;
	math::Rect<ScalarDistanceF> m_Margin;
	ScalarDimensionF m_Size;
	math::Dimension2F m_MinSize;
	math::Dimension2F m_MaxSize;

	u32 m_TabId;

	bool m_CanFocus;
	bool m_IsVisible;
	bool m_IsEnabled;
	bool m_NoClip;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_ELEMENT_H