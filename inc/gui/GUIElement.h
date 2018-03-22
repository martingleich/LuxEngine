#ifndef INCLUDED_GUI_ELEMENT_H
#define INCLUDED_GUI_ELEMENT_H
#include "core/Referable.h"
#include "core/lxArray.h"

#include "math/Rect.h"

#include "gui/GUISkin.h"
#include "gui/GUIPalette.h"
#include "gui/GUIScalarDistance.h"
#include "gui/GUIAlign.h"

namespace lux
{
namespace gui
{

class Font;
class Event;
class MouseEvent;
class KeyboardEvent;
class ElementEvent;
class Window;
class Renderer;
class GUIEnvironment;

class Element : public Referable
{
public:
	using ElementList = core::Array<StrongRef<Element>>;
	using ElementIterator = ElementList::Iterator;
	using ConstElementIterator = ElementList::ConstIterator;

public:
	LUX_API static const int NO_TAB_STOP;
	LUX_API static const ScalarDistanceF AUTO_SIZE;
	LUX_API static const ScalarDistanceF AUTO_MARGIN;
	LUX_API static const ScalarDistanceF AUTO_OUT_MARGIN;

public:
	LUX_API Element();
	LUX_API virtual ~Element();

	LUX_API Element* GetParent() const;

	LUX_API Window* GetWindow() const;
	LUX_API GUIEnvironment* GetEnvironment() const;
	LUX_API virtual void SetEnvironment(GUIEnvironment* env);

	LUX_API virtual void SetTabId(int tabId);
	LUX_API virtual int GetTabId() const;
	LUX_API bool IsTabStop() const;

	LUX_API virtual bool IsFocusable() const;
	LUX_API virtual void SetFocusable(bool focus);
	LUX_API virtual bool IsFocused() const;

	LUX_API virtual bool IsHovered() const;

	LUX_API virtual bool IsTrulyVisible() const;
	LUX_API virtual bool IsVisible() const;
	LUX_API virtual void SetVisible(bool visible);

	LUX_API virtual bool IsEnabled() const;
	LUX_API virtual void SetEnabled(bool enable);

	LUX_API virtual bool IsClipped() const;
	LUX_API virtual void SetClipping(bool clip);

	virtual const core::String& GetText() const { return core::String::EMPTY; }
	virtual void SetText(const core::String& text) { LUX_UNUSED(text); }

	LUX_API virtual const core::String& GetToolTip() const;
	LUX_API virtual void SetToolTip(const core::String& tip);

	LUX_API virtual void SetMinSize(const math::Dimension2F& minSize);
	LUX_API virtual const math::Dimension2F& GetMinSize() const;
	LUX_API virtual void SetBorder(const math::RectF& border);
	LUX_API virtual const math::RectF& GetBorder() const;

	LUX_API virtual const ScalarDimensionF GetSize() const;
	LUX_API ScalarDistanceF GetWidth() const;
	LUX_API ScalarDistanceF GetHeight() const;
	LUX_API virtual void SetSize(const ScalarDimensionF& size);
	LUX_API void SetSize(ScalarDistanceF width, ScalarDistanceF heigth);
	LUX_API void SetWidth(ScalarDistanceF width);
	LUX_API void SetHeight(ScalarDistanceF width);
	LUX_API virtual void SetInnerSize(const ScalarDimensionF& size);

	LUX_API virtual const math::Rect<ScalarDistanceF>& GetMargin() const;
	LUX_API virtual void SetMargin(const math::Rect<ScalarDistanceF>& rect);
	LUX_API void SetMargin(ScalarDistanceF left, ScalarDistanceF top,
		ScalarDistanceF right, ScalarDistanceF bottom);

	LUX_API virtual void SetPlacement(const math::Rect<ScalarDistanceF>& margin, const ScalarDimensionF& size);

	LUX_API void SetPosition(const ScalarVectorF & pos);
	LUX_API void SetPosition(ScalarDistanceF x, ScalarDistanceF y);
	LUX_API math::Vector2F GetPosition() const;

	LUX_API core::Range<ElementIterator> Elements();
	LUX_API core::Range<ConstElementIterator> Elements() const;

	LUX_API virtual void RemoveElement(ElementIterator it);
	LUX_API void RemoveElement(Element* elem);
	LUX_API virtual void RemoveAllElements();

	LUX_API StrongRef<Element> AddElement(Element* elem);
	LUX_API virtual StrongRef<Element> AddElement(Element* elem, ElementIterator where);

	LUX_API virtual math::RectF GetFinalInnerRect() const;
	LUX_API math::Dimension2F GetFinalInnerSize() const;
	LUX_API float GetFinalInnerWidth() const;
	LUX_API float GetFinalInnerHeight() const;

	LUX_API virtual math::RectF GetFinalRect() const;
	LUX_API math::Dimension2F GetFinalSize() const;
	LUX_API float GetFinalWidth() const;
	LUX_API float GetFinalHeight() const;

	//! Handle a gui event
	/**
	\param e The event to handle
	\return True if the event was handled otherwise false.
	*/
	LUX_API virtual bool OnEvent(const Event& e);
	virtual bool OnMouseEvent(const gui::MouseEvent& e) { LUX_UNUSED(e); return false; }
	virtual bool OnKeyboardEvent(const gui::KeyboardEvent& e) { LUX_UNUSED(e); return false; }
	virtual bool OnElementEvent(const gui::ElementEvent& e) { LUX_UNUSED(e); return false; }

	virtual void Paint(Renderer* renderer) { LUX_UNUSED(renderer); }
	virtual void Paint(Renderer* renderer, float secsPassed) { LUX_UNUSED(secsPassed); Paint(renderer); }

	LUX_API virtual void SetOverwriteSkin(Skin* s);
	LUX_API virtual Skin* GetOverwriteSkin() const;
	LUX_API virtual Skin* GetSkin() const;

	LUX_API virtual void SetFont(Font* f);
	LUX_API virtual Font* GetFont() const;

	LUX_API void SetAlignment(EAlign align);
	LUX_API EAlign GetAlignment() const;

	LUX_API virtual ECursorState GetHoverCursor() const;

	LUX_API virtual void SetPalette(const Palette& palette);
	LUX_API virtual const Palette& GetPalette() const;
	LUX_API virtual Palette GetFinalPalette() const;
	LUX_API virtual void SetTextColor(video::Color c);
	LUX_API virtual void SetBackgroundColor(video::Color c);

	LUX_API virtual EGUIState GetState() const;

	LUX_API virtual bool IsPointInside(const math::Vector2F& point) const;

	StrongRef<gui::Element> Clone() const
	{
		return CloneImpl().StaticCastStrong<gui::Element>();
	}

protected:
	LUX_API virtual void SetSkin(Skin* s);

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

	EAlign m_Align;
	StrongRef<Skin> m_OverwriteSkin;
	StrongRef<Skin> m_Skin;

	StrongRef<Font> m_OverwriteFont;
	Palette m_Palette;

	GUIEnvironment* m_Environment;
	Element* m_Parent;
	Window* m_Window;

	core::String m_ToolTip;

	mutable math::RectF m_FinalRect;
	mutable math::RectF m_InnerRect;

	math::RectF m_Border;
	math::Rect<ScalarDistanceF> m_Margin;
	ScalarDimensionF m_Size;
	math::Dimension2F m_MinSize;

	int m_TabId;

	bool m_CanFocus : 1;
	bool m_IsVisible : 1;
	bool m_IsEnabled : 1;
	bool m_NoClip : 1;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_ELEMENT_H