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
	LUX_API Color GetPixel(s32 x, s32 y);

	void SetFastPixelCheck(s32 x, s32 y, u8 native_color[4])
	{
		if(x < 0 || x >= m_Width)
			return;
		if(y < 0 || y >= m_Height)
			return;

		SetFastPixel(x, y, native_color);
	}

	void SetFastPixel(s32 x, s32 y, u8 native_color[4])
	{
		u32 b = m_Format.GetBytePerPixel();
		memcpy(m_Data + y*m_Pitch + b * x, native_color, b);
	}

	void SetPixelUnchecked(s32 x, s32 y, Color color)
	{
		m_Format.A8R8G8B8ToFormat((u32)color, m_Data + y*m_Pitch + x*m_Format.GetBytePerPixel());
	}

	Color GetPixelUnchecked(s32 x, s32 y)
	{
		return m_Format.FormatToA8R8G8B8(m_Data + y*m_Pitch + x*m_Format.GetBytePerPixel());
	}

	void DrawAlphaPixel(s32 x, s32 y, s32 blend, Color color)
	{
		if(x < 0)
			return;
		if(y < 0)
			return;
		if(x >= m_Width)
			return;
		if(y >= m_Height)
			return;

		blend = 255 - blend;

		Color c0 = GetPixelUnchecked(x, y);
		u32 r0 = c0.GetRed();
		u32 g0 = c0.GetGreen();
		u32 b0 = c0.GetBlue();

		u32 r1 = color.GetRed();
		u32 g1 = color.GetGreen();
		u32 b1 = color.GetBlue();

		u32 r = (r1*blend + r0*(255 - blend)) / 255;
		u32 g = (g1*blend + g0*(255 - blend)) / 255;
		u32 b = (b1*blend + b0*(255 - blend)) / 255;
		SetPixelUnchecked(x, y, Color(r, g, b));
	}

	template <typename MapFunc>
	void ApplyMapping(const MapFunc& func)
	{
		if(m_Format == ColorFormat::A8R8G8B8) {
			for(s32 y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(s32 x = 0; x < m_Width; ++x) {
					func(cur[3], cur[2], cur[1], cur[0],
						cur[3], cur[2], cur[1], cur[0]);
					cur += 4;
				}
			}
		} else if(m_Format == ColorFormat::R8G8B8) {
			for(s32 y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(s32 x = 0; x < m_Width; ++x) {
					u8 dummyAlpha;
					func(0, cur[2], cur[1], cur[0],
						dummyAlpha, cur[2], cur[1], cur[0]);
					cur += 3;
				}
			}
		} else {
			for(s32 y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(s32 x = 0; x < m_Width; ++x) {
					Color px = m_Format.FormatToA8R8G8B8(cur);
					u8 data[4];
					func(px.GetAlpha(), px.GetRed(), px.GetGreen(), px.GetBlue(),
						data[0], data[1], data[2], data[3]);
					px.SetAlpha(data[0]);
					px.SetRed(data[1]);
					px.SetGreen(data[2]);
					px.SetBlue(data[3]);
					m_Format.A8R8G8B8ToFormat((u32)px, cur);
					cur += m_Format.GetBytePerPixel();
				}
			}
		}
	}

	struct GreyMapping
	{
		inline void operator()(
			u8 a, u8 r, u8 g, u8 b,
			u8& _a, u8& _r, u8& _g, u8& _b) const
		{
			u8 bright = (u8)((r * 299 + g * 587 + b * 114) / 1000);
			_a = a;
			_r = bright;
			_g = bright;
			_b = bright;
		}
	};

	struct ChromaKeyMapping
	{
		ChromaKeyMapping(Color chroma) :
			key_r((u8)chroma.GetRed()),
			key_g((u8)chroma.GetGreen()),
			key_b((u8)chroma.GetBlue())
		{
		}

		inline void operator()(
			u8, u8 r, u8 g, u8 b,
			u8& _a, u8& _r, u8&, u8&) const
		{
			if(r == key_r && g == key_g && b == key_b)
				_a = 0;
			else
				_a = 255;
		}

		u8 key_r;
		u8 key_g;
		u8 key_b;
	};

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
