#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H
#include "video/ColorSpaces.h"
#include "video/ColorInt.h"
#include "video/ColorFloat.h"
#include "video/ColorFormat.h"

namespace lux
{
namespace video
{

//! Convert a colorf to hsv
inline void ColorfToHSV(const ColorF& f, float& h, float& s, float& v)
{
	RGBToHSV(f.r, f.g, f.b, h, s, v);
}

//! Convert a hsv color to colorf
inline ColorF HSVToColorf(float h, float s, float v, float alpha = 1.0f)
{
	return ColorF::FromHSV(h, s, v, alpha);
}

//! Cast floating point color to integer color.
/**
Warning: Lossy
All values outside the [0, 1] range are clamped.
*/
inline Color ColorFToColor(const ColorF& c)
{
	return Color(
		(u32)math::Clamp(c.r * 255.0f, 0.0f, 255.0f),
		(u32)math::Clamp(c.g * 255.0f, 0.0f, 255.0f),
		(u32)math::Clamp(c.b * 255.0f, 0.0f, 255.0f),
		(u32)math::Clamp(c.a * 255.0f, 0.0f, 255.0f));
}

}    // video
}    // lux

#endif
