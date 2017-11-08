#ifndef INCLUDED_GUI_GUIALIGN_H
#define INCLUDED_GUI_GUIALIGN_H
#include "math/Rect.h"

namespace lux
{
namespace gui
{

//! Enumeration giving the alignment of text or object.
enum class EAlign
{
	HLeft = 1,          //!< Align to the left
	HCenter = 2,        //!< Align to the horicontal center
	HRight = 4,         //!< Align to the right

	VBottom = 8,        //!< Align to the bottom
	VCenter = 16,       //!< Align to the vertical center
	VTop = 32,          //!< Align to the top

	Centered = VCenter | HCenter,        //!< Align text in the center
	BottomLeft = VBottom | HLeft,       //!< Align to the bottom left
	TopLeft = VTop | HLeft, //!< Align to the top left
	BottomRight = VBottom | HRight,       //!< Align to the bottom right
	TopRight = VTop | HRight, //!< Align to the top right
};

}
DECLARE_FLAG_CLASS(lux::gui::EAlign);
namespace gui
{

inline EAlign FlipAlign(EAlign in, bool flipH, bool flipV)
{
	if(flipH) {
		if(TestFlag(in, EAlign::HLeft)) {
			in |= EAlign::HRight;
			in &= EAlign::HLeft;
		} else if(TestFlag(in, EAlign::HRight)) {
			in |= EAlign::HLeft;
			in &= EAlign::HRight;
		}
	}
	if(flipV) {
		if(TestFlag(in, EAlign::VTop)) {
			in |= EAlign::VBottom;
			in &= EAlign::VTop;
		} else if(TestFlag(in, EAlign::VBottom)) {
			in |= EAlign::VTop;
			in &= EAlign::VBottom;
		}
	}
	return in;
}

template <typename T>
math::Rect<T> AlignRect(
	T w, T h,
	const math::Rect<T>& base,
	EAlign align)
{
	math::RectF out;
	if(TestFlag(align, EAlign::HLeft)) {
		out.left = base.left;
		out.right = out.left + w;
	} else if(TestFlag(align, EAlign::HRight)) {
		out.right = base.right;
		out.left = out.right - w;
	} else {
		auto c = (base.left + base.right) / 2;
		out.left = c - w / 2;
		out.right = out.left + w;
	}
	if(TestFlag(align, EAlign::VTop)) {
		out.top = base.top;
		out.bottom = out.top + h;
	} else if(TestFlag(align, EAlign::HRight)) {
		out.bottom = base.bottom;
		out.top = out.bottom - h;
	} else {
		auto c = (base.top + base.bottom) / 2;
		out.top = c - h / 2;
		out.bottom = out.top + h;
	}

	return out;
}

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_GUIALIGN_H

