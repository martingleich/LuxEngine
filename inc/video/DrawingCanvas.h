#ifndef INCLUDED_DRAWING_CANVAS_H
#define INCLUDED_DRAWING_CANVAS_H
#include "math/rect.h"
#include "video/ColorFormat.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

class DrawingCanvas
{
public:
	LUX_API DrawingCanvas();
	LUX_API DrawingCanvas(void* d, ColorFormat f, const math::dimension2du& dim, u32 p);
	LUX_API ~DrawingCanvas();
	LUX_API bool IsValid();
	LUX_API void Clear(Color color);
	LUX_API void SetPixel(s32 x, s32 y, Color color);
	LUX_API void DrawHLine(s32 y, s32 left, s32 right, Color color);
	LUX_API void DrawVLine(s32 x, s32 top, s32 bottom, Color color);
	LUX_API void DrawLine(s32 x0, s32 y0, s32 x1, s32 y1, Color color);
	LUX_API void DrawRectangle(s32 left, s32 top, s32 right, s32 bottom, Color color);
	// Draw a rectangle in range [left, right] and [top, bottom];
	LUX_API void FillRectangle(s32 left, s32 top, s32 right, s32 bottom, Color color);
	LUX_API void DrawEllipse(s32 xm, s32 ym, s32 a, s32 b, Color color);
	LUX_API void FillEllipse(s32 xm, s32 ym, s32 a, s32 b, Color color);
	LUX_API void FillTriangle(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color color);
	LUX_API u32 GetWidth() const;
	LUX_API u32 GetHeight() const;
	LUX_API void* GetRaw();
	LUX_API ColorFormat GetFormat() const;
	LUX_API u32 GetPitch() const;
private:
	LUX_API s32 orient2d(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2);
	LUX_API u8 ComputeOutCode(s32 x, s32 y, const math::recti& r);
	LUX_API bool ClipLine(s32& x0, s32& y0, s32& x1, s32& y1, const math::recti& r);
	LUX_API bool ClipRectangle(s32& left, s32& top, s32& right, s32& bottom, const math::recti&r);
	LUX_API void SetFastPixelCheck(s32 x, s32 y, u8 native_color[4]);
	LUX_API void SetFastPixel(s32 x, s32 y, u8 native_color[4]);
	LUX_API void SetPixelUnchecked(s32 x, s32 y, Color color);
	LUX_API Color GetPixelUnchecked(s32 x, s32 y);
	LUX_API void DrawAlphaPixel(s32 x, s32 y, s32 blend, Color color);

private:
	u8* m_Data;
	ColorFormat m_Format;
	s32 m_Width;
	s32 m_Height;

	math::recti m_Rect;

	u32 m_Pitch;
};

template <typename T>
class DrawingCanvasAuto;

}
}

#endif // !INCLUDED_DRAWING_CANVAS_H
