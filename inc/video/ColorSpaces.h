#ifndef INCLUDED_LUX_COLOR_SPACES_H
#define INCLUDED_LUX_COLOR_SPACES_H
#include "math/lxMath.h"

namespace lux
{
namespace video
{
inline void RGBToHSV(float r, float g, float b, float& h, float& s, float& v)
{
	float cmax = math::Max(r, g, b);
	float cmin = math::Min(r, g, b);
	float delta = cmax - cmin;

	if(math::IsZero(delta)) {
		h = 0;
	} else {
		if(cmax == r)
			h = fmodf(((g - b) / delta), 6);
		else if(cmax == g)
			h = ((b - r) / delta) + 2;
		else if(cmax == b)
			h = ((r - g) / delta) + 4;
		h *= 1.0f / 6.0f;

		if(h < 0)
			h += 1;
	}

	if(math::IsZero(cmax))
		s = 0;
	else 
		s = delta / cmax;

	v = cmax;
}

inline void HSVToRGB(float h, float s, float v, float& r, float& g, float& b)
{
	float c = v * s;
	float x = c * (1 - fabsf(fmodf(h*6.0f, 2) - 1));
	float m = v - c;

	int hprime = (int)fmodf(h * 6.0f, 6);
	switch(hprime) {
	case 0:  r = c; g = x; b = 0; break;
	case 1:  r = x; g = c; b = 0; break;
	case 2:  r = 0; g = c; b = x; break;
	case 3:  r = 0; g = x; b = c; break;
	case 4:  r = x; g = 0; b = c; break;
	case 5:  r = c; g = 0; b = x; break;
	default: r = 0; g = 0; b = 0; break;
	}

	r += m;
	g += m;
	b += m;
}

}
}

#endif // #ifndef INCLUDED_LUX_COLOR_SPACES_H