#ifndef INCLUDED_DRAWING_CANVAS_H
#define INCLUDED_DRAWING_CANVAS_H
#include "math/Rect.h"
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
	LUX_API DrawingCanvas(void* d, ColorFormat f, const math::Dimension2I& dim, int p);
	LUX_API ~DrawingCanvas();

	LUX_API bool IsValid();

	LUX_API void Clear(Color color);

	LUX_API void SetPixel(int x, int y, Color color);
	LUX_API Color GetPixel(int x, int y);

	void SetFastPixelCheck(int x, int y, u8 native_color[4])
	{
		if(x < 0 || x >= m_Width)
			return;
		if(y < 0 || y >= m_Height)
			return;

		SetFastPixel(x, y, native_color);
	}

	void SetFastPixel(int x, int y, u8 native_color[4])
	{
		int b = m_Format.GetBytePerPixel();
		memcpy(m_Data + y*m_Pitch + b * x, native_color, b);
	}

	void SetPixelUnchecked(int x, int y, Color color)
	{
		m_Format.A8R8G8B8ToFormat(color.ToDWORD(), m_Data + y*m_Pitch + x*m_Format.GetBytePerPixel());
	}

	Color GetPixelUnchecked(int x, int y)
	{
		return m_Format.FormatToA8R8G8B8(m_Data + y*m_Pitch + x*m_Format.GetBytePerPixel());
	}

	void DrawAlphaPixel(int x, int y, int blend, Color color)
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
		int r0 = c0.GetRed();
		int g0 = c0.GetGreen();
		int b0 = c0.GetBlue();

		int r1 = color.GetRed();
		int g1 = color.GetGreen();
		int b1 = color.GetBlue();

		int r = (r1*blend + r0*(255 - blend)) / 255;
		int g = (g1*blend + g0*(255 - blend)) / 255;
		int b = (b1*blend + b0*(255 - blend)) / 255;
		SetPixelUnchecked(x, y, Color(r, g, b));
	}

	template <typename MapFunc>
	void ApplyMapping(const MapFunc& func)
	{
		if(m_Format == ColorFormat::A8R8G8B8) {
			for(int y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(int x = 0; x < m_Width; ++x) {
					func(cur[3], cur[2], cur[1], cur[0],
						cur[3], cur[2], cur[1], cur[0]);
					cur += 4;
				}
			}
		} else if(m_Format == ColorFormat::R8G8B8) {
			for(int y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(int x = 0; x < m_Width; ++x) {
					u8 dummyAlpha;
					func(0, cur[2], cur[1], cur[0],
						dummyAlpha, cur[2], cur[1], cur[0]);
					cur += 3;
				}
			}
		} else {
			for(int y = 0; y < m_Height; ++y) {
				u8* cur = m_Data + y*m_Pitch;
				for(int x = 0; x < m_Width; ++x) {
					Color px = m_Format.FormatToA8R8G8B8(cur);
					u8 data[4];
					func(px.GetAlpha(), px.GetRed(), px.GetGreen(), px.GetBlue(),
						data[0], data[1], data[2], data[3]);
					px.SetAlpha(data[0]);
					px.SetRed(data[1]);
					px.SetGreen(data[2]);
					px.SetBlue(data[3]);
					m_Format.A8R8G8B8ToFormat((int)px, cur);
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
			u8& _a, u8&, u8&, u8&) const
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

	LUX_API void DrawHLine(int y, int left, int right, Color color);
	LUX_API void DrawVLine(int x, int top, int bottom, Color color);
	LUX_API void DrawLine(int x0, int y0, int x1, int y1, Color color);
	LUX_API void DrawRectangle(int left, int top, int right, int bottom, Color color);

	// Draw a rectangle in range [left, right] and [top, bottom];
	LUX_API void FillRectangle(int left, int top, int right, int bottom, Color color);
	LUX_API void DrawEllipse(int xm, int ym, int a, int b, Color color);
	LUX_API void FillEllipse(int xm, int ym, int a, int b, Color color);
	LUX_API void FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color);

	LUX_API int GetWidth() const;
	LUX_API int GetHeight() const;
	LUX_API void* GetRaw();
	LUX_API ColorFormat GetFormat() const;
	LUX_API int GetPitch() const;

private:
	LUX_API int orient2d(int x0, int y0, int x1, int y1, int x2, int y2);
	LUX_API u8 ComputeOutCode(int x, int y, const math::RectI& r);
	LUX_API bool ClipLine(int& x0, int& y0, int& x1, int& y1, const math::RectI& r);
	LUX_API bool ClipRectangle(int& left, int& top, int& right, int& bottom, const math::RectI&r);

private:
	u8* m_Data;
	ColorFormat m_Format;
	int m_Width;
	int m_Height;

	math::RectI m_Rect;

	int m_Pitch;
};

template <typename T>
class DrawingCanvasAuto;

}
}

#endif // !INCLUDED_DRAWING_CANVAS_H
