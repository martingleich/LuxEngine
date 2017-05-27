#ifndef INCLUDED_DRAWING_CANVAS_H
#define INCLUDED_DRAWING_CANVAS_H
#include "math/rect.h"
#include "video/images/Image.h"
#include "video/Texture.h"

namespace lux
{
namespace video
{
class DrawingCanvas
{
public:
	DrawingCanvas() :
		m_Data(nullptr), m_Width(0), m_Height(0), m_Pitch(0)
	{
		m_Rect.top = 0;
		m_Rect.left = 0;
		m_Rect.bottom = m_Height - 1;
		m_Rect.right = m_Width - 1;
	}

	DrawingCanvas(void* d, ColorFormat f, const math::dimension2du& dim, u32 p) :
		m_Data((u8*)d), m_Format(f), m_Width(dim.width), m_Height(dim.height), m_Pitch(p)
	{
		m_Rect.top = 0;
		m_Rect.left = 0;
		m_Rect.bottom = m_Height - 1;
		m_Rect.right = m_Width - 1;
	}

	bool IsValid()
	{
		if(!m_Data)
			return false;
		if(m_Width == 0 || m_Height == 0)
			return false;
		if(m_Pitch < m_Width * m_Format.GetBytePerPixel())
			return false;
		if(m_Format == ColorFormat::UNKNOWN || m_Format.IsFloatingPoint())
			return false;

		return true;
	}

	void Clear(Color color)
	{
		FillRectangle(0, 0, m_Width, m_Height, color);
	}

	void SetPixel(s32 x, s32 y, Color color)
	{
		if(x < 0 || x >= m_Width)
			return;
		if(y < 0 || y >= m_Height)
			return;

		SetPixelUnchecked(x, y, color);
	}

	void DrawHLine(s32 y, s32 left, s32 right, Color color)
	{
		if(y < 0 || y >= m_Height)
			return;

		left = math::Clamp<s32>(left, 0, m_Width);
		right = math::Clamp<s32>(right, 0, m_Width);

		if(right < left)
			return;

		u32 width = right - left;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);
		u32 bytes = m_Format.GetBytePerPixel();

		u8* cur = m_Data + y*m_Pitch + left*bytes;

		for(u32 i = 0; i < width; ++i) {
			memcpy(cur, native_color, bytes);
			cur += bytes;
		}
	}

	void DrawVLine(s32 x, s32 top, s32 bottom, Color color)
	{
		if(x < 0 || x >= m_Width)
			return;

		top = math::Clamp<s32>(top, 0, m_Height);
		bottom = math::Clamp<s32>(bottom, 0, m_Height);

		if(bottom < top)
			return;

		u32 height = bottom - top;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);
		u32 bytes = m_Format.GetBytePerPixel();

		u8* cur = m_Data + top*m_Pitch + x*bytes;
		for(u32 i = 0; i < height; ++i) {
			memcpy(cur, native_color, bytes);
			cur += m_Pitch;
		}
	}

	void DrawLine(s32 x0, s32 y0, s32 x1, s32 y1, Color color)
	{
		if(!ClipLine(x0, y0, x1, y1, m_Rect))
			return;

		// Make definition range smaller, this will ensure no overflows in the error variables.
		x0 -= x1;
		//x1 = 0; Implicit done
		y0 -= y1;
		//y1 = 0; Implicit done

		s32 diffx = 0 - x0;
		s32 dx, sx;
		if(diffx >= 0) {
			sx = 1;
			dx = diffx;
		} else {
			sx = -1;
			dx = -diffx;
		}

		s32 diffy = 0 - y0;
		s32 dy, sy;
		if(diffy >= 0) {
			sy = 1;
			dy = -diffy;
		} else {
			sy = -1;
			dy = diffy;
		}

		s32 err = dx + dy;
		s32 e2;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);

		SetFastPixel(x0 + x1, y0 + y1, native_color);
		do {
			e2 = 2 * err;
			if(e2 > dy) {
				err += dy;
				x0 += sx;
			}
			if(e2 < dx) {
				err += dx;
				y0 += sy;
			}
			SetFastPixel(x0 + x1, y0 + y1, native_color);
		} while(x0 != 0 || y0 != 0);
	}

	void DrawRectangle(s32 left, s32 top, s32 right, s32 bottom, Color color)
	{
		DrawHLine(top, left, right, color);
		DrawHLine(bottom, left, right, color);
		DrawVLine(left, top, bottom, color);
		DrawVLine(right, top, bottom, color);
	}

	// Draw a rectangle in range [left, right] and [top, bottom]
	void FillRectangle(s32 left, s32 top, s32 right, s32 bottom, Color color)
	{
		if(!ClipRectangle(left, top, right, bottom, m_Rect))
			return;

		u32 width = right - left + 1;
		u32 height = bottom - top + 1;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);
		u32 bytes = m_Format.GetBytePerPixel();

		u8* base = m_Data + top*m_Pitch + left*bytes;
		u8* cur = base;

		// Write first line
		for(u32 i = 0; i < width; ++i) {
			memcpy(cur, native_color, bytes);
			cur += bytes;
		}

		cur = base + m_Pitch;

		// Copy lines
		for(u32 i = 1; i < height; ++i) {
			memcpy(cur, base, width*bytes);
			cur += m_Pitch;
		}
	}

	void DrawEllipse(s32 xm, s32 ym, s32 a, s32 b, Color color)
	{
		if(a == 0 && b == 0)
			return;

		s32 dx = 0, dy = b;
		s32 a2 = a*a, b2 = b*b;
		s32 err = b2 - (2 * b - 1)*a2;
		s32 e2;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);

		do {
			SetFastPixelCheck(xm + dx, ym + dy, native_color);
			SetFastPixelCheck(xm - dx, ym + dy, native_color);
			SetFastPixelCheck(xm - dx, ym - dy, native_color);
			SetFastPixelCheck(xm + dx, ym - dy, native_color);

			e2 = 2 * err;
			if(e2 < (2 * dx + 1)*b2) {
				dx++;
				err += (2 * dx + 1)*b2;
			}
			if(e2 > -(2 * dy - 1)*a2) {
				dy--;
				err -= (2 * dy - 1)*a2;
			}
		} while(dy >= 0);

		while(dx++ < a) {
			SetFastPixelCheck(xm + dx, ym, native_color);
			SetFastPixelCheck(xm - dx, ym, native_color);
		}
	}

	void FillEllipse(s32 xm, s32 ym, s32 a, s32 b, Color color)
	{
		if(a == 0 && b == 0)
			return;

		s32 dx = 0, dy = b;
		s32 a2 = a*a, b2 = b*b;
		s32 err = b2 - (2 * b - 1)*a2;
		s32 e2;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);

		do {
			DrawHLine(ym + dy, xm - dx, xm + dx, color);
			DrawHLine(ym - dy, xm - dx, xm + dx, color);

			e2 = 2 * err;
			if(e2 < (2 * dx + 1)*b2) {
				dx++;
				err += (2 * dx + 1)*b2;
			}
			if(e2 > -(2 * dy - 1)*a2) {
				dy--;
				err -= (2 * dy - 1)*a2;
			}
		} while(dy >= 0);

		while(dx++ < a) {
			DrawHLine(ym, xm - dx, xm + dx, color);
		}
	}

	void FillTriangle(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color color)
	{
		s32 minX = math::Min(x0, x1, x2);
		s32 minY = math::Min(y0, y1, y2);
		s32 maxX = math::Max(x0, x1, x2);
		s32 maxY = math::Max(y0, y1, y2);

		if(!ClipRectangle(minX, minY, maxX, maxY, m_Rect))
			return;

		u8 native_color[4];
		m_Format.A8R8G8B8ToFormat((u32)color, native_color);

		s32 A01 = y0 - y1, B01 = x1 - x0;
		s32 A20 = y2 - y0, B20 = x0 - x2;

		if(-B20 * A01 + A20 * B01 < 0) {
			s32 t;
			t = x1; x1 = x2; x2 = t;
			t = y1; y1 = y2; y2 = t;

			A01 = y0 - y1; B01 = x1 - x0;
			A20 = y2 - y0; B20 = x0 - x2;
		}

		s32 A12 = y1 - y2, B12 = x2 - x1;

		s32 w0r = orient2d(x1, y1, x2, y2, minX, minY);
		s32 w1r = orient2d(x2, y2, x0, y0, minX, minY);
		s32 w2r = orient2d(x0, y0, x1, y1, minX, minY);

		for(s32 y = minY; y <= maxY; ++y) {
			s32 w0 = w0r;
			s32 w1 = w1r;
			s32 w2 = w2r;

			for(s32 x = minX; x <= maxX; ++x) {
				if(w0 >= 0 && w1 >= 0 && w2 >= 0)
					SetFastPixel(x, y, native_color);

				w0 += A12;
				w1 += A20;
				w2 += A01;
			}

			w0r += B12;
			w1r += B20;
			w2r += B01;
		}
	}

	u32 GetWidth() const
	{
		return m_Width;
	}

	u32 GetHeight() const
	{
		return m_Height;
	}

	void* GetRaw()
	{
		return m_Data;
	}

	ColorFormat GetFormat() const
	{
		return m_Format;
	}

	u32 GetPitch() const
	{
		return m_Pitch;
	}

private:
	s32 orient2d(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2)
	{
		return (x1 - x0)*(y2 - y0) - (y1 - y0)*(x2 - x0);
	}

	u8 ComputeOutCode(s32 x, s32 y, const math::recti& r)
	{
		u8 code = 0;

		if(x < r.left)
			code |= 1;
		if(x > r.right)
			code |= 2;
		if(y < r.top)
			code |= 4;
		if(y > r.bottom)
			code |= 8;

		return code;
	}

	bool ClipLine(s32& x0, s32& y0, s32& x1, s32& y1, const math::recti& r)
	{
		u8 out0 = ComputeOutCode(x0, y0, r);
		u8 out1 = ComputeOutCode(x1, y1, r);

		while(true) {
			if(!(out0 | out1)) {
				return true;
			} else if(out0 & out1) {
				return false;
			} else {
				int x, y;
				u8 out = out0 ? out0 : out1;
				if(out & 8) {
					x = x0 + ((x1 - x0) * (r.bottom - y0)) / (y1 - y0);
					y = r.bottom;
				} else if(out & 4) {
					x = x0 + ((x1 - x0) * (r.top - y0)) / (y1 - y0);
					y = r.top;
				} else if(out & 2) {
					y = y0 + ((y1 - y0) * (r.right - x0)) / (x1 - x0);
					x = r.right;
				} else if(out & 1) {
					y = y0 + ((y1 - y0) * (r.left - x0)) / (x1 - x0);
					x = r.left;
				}

				if(out == out0) {
					x0 = x;
					y0 = y;
					out0 = ComputeOutCode(x0, y0, r);
				} else {
					x1 = x;
					y1 = y;
					out1 = ComputeOutCode(x1, y1, r);
				}
			}
		}
	}

	bool ClipRectangle(s32& left, s32& top, s32& right, s32& bottom, const math::recti&r)
	{
		if(left < r.left)
			left = r.left;
		if(top < r.top)
			top = r.top;
		if(right > r.right)
			right = r.right;
		if(bottom > r.bottom)
			bottom = r.bottom;

		if(right < left)
			return false;
		if(bottom < top)
			return false;

		return true;
	}

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

private:
	u8* m_Data;
	ColorFormat m_Format;
	s32 m_Width;
	s32 m_Height;

	math::recti m_Rect;

	u32 m_Pitch;
};

template <typename LockType>
struct DrawingCanvasScope
{
	DrawingCanvasScope(LockType&& _lock) :
		lock(std::move(_lock)),
		canvas(lock.data, lock.base->GetColorFormat(), lock.base->GetSize(), lock.pitch)
	{
	}

	LockType lock;
	DrawingCanvas canvas;
};

}
}

#endif // !INCLUDED_DRAWING_CANVAS_H
