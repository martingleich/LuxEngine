#include "video/DrawingCanvas.h"	

namespace lux
{
namespace video
{
DrawingCanvas::DrawingCanvas() :
	m_Data(nullptr), m_Width(0), m_Height(0), m_Pitch(0)
{
	m_Rect.top = 0;
	m_Rect.left = 0;
	m_Rect.bottom = m_Height - 1;
	m_Rect.right = m_Width - 1;
}

DrawingCanvas::DrawingCanvas(void* d, ColorFormat f, const math::Dimension2I& dim, int p) :
	m_Data((u8*)d), m_Format(f), m_Width(dim.width), m_Height(dim.height), m_Pitch(p)
{
	if(m_Format.IsCompressed())
		throw core::InvalidOperationException("Can't use DrawingCanvas with compressed formats");

	m_Rect.top = 0;
	m_Rect.left = 0;
	m_Rect.bottom = m_Height - 1;
	m_Rect.right = m_Width - 1;
}

DrawingCanvas::~DrawingCanvas()
{
}

bool DrawingCanvas::IsValid()
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

void DrawingCanvas::Clear(Color color)
{
	FillRectangle(0, 0, m_Width, m_Height, color);
}

void DrawingCanvas::SetPixel(int x, int y, Color color)
{
	if(x < 0 || x >= m_Width)
		return;
	if(y < 0 || y >= m_Height)
		return;

	SetPixelUnchecked(x, y, color);
}

Color DrawingCanvas::GetPixel(int x, int y)
{
	if(x < 0 || x >= m_Width)
		return 0;
	if(y < 0 || y >= m_Height)
		return 0;

	return GetPixelUnchecked(x, y);
}

void DrawingCanvas::DrawHLine(int y, int left, int right, Color color)
{
	if(y < 0 || y >= m_Height)
		return;

	left = math::Clamp<int>(left, 0, m_Width);
	right = math::Clamp<int>(right, 0, m_Width);

	if(right < left)
		return;

	int width = right - left;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);
	int bytes = m_Format.GetBytePerPixel();

	u8* cur = m_Data + y*m_Pitch + left*bytes;

	for(int i = 0; i < width; ++i) {
		std::memcpy(cur, native_color, bytes);
		cur += bytes;
	}
}

void DrawingCanvas::DrawVLine(int x, int top, int bottom, Color color)
{
	if(x < 0 || x >= m_Width)
		return;

	top = math::Clamp<int>(top, 0, m_Height);
	bottom = math::Clamp<int>(bottom, 0, m_Height);

	if(bottom < top)
		return;

	int height = bottom - top;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);
	int bytes = m_Format.GetBytePerPixel();

	u8* cur = m_Data + top*m_Pitch + x*bytes;
	for(int i = 0; i < height; ++i) {
		std::memcpy(cur, native_color, bytes);
		cur += m_Pitch;
	}
}

void DrawingCanvas::DrawLine(int x0, int y0, int x1, int y1, Color color)
{
	if(!ClipLine(x0, y0, x1, y1, m_Rect))
		return;

	// Make definition range smaller, this will ensure no overflows in the error variables.
	x0 -= x1;
	//x1 = 0; Implicit done
	y0 -= y1;
	//y1 = 0; Implicit done

	int diffx = 0 - x0;
	int dx, sx;
	if(diffx >= 0) {
		sx = 1;
		dx = diffx;
	} else {
		sx = -1;
		dx = -diffx;
	}

	int diffy = 0 - y0;
	int dy, sy;
	if(diffy >= 0) {
		sy = 1;
		dy = -diffy;
	} else {
		sy = -1;
		dy = diffy;
	}

	int err = dx + dy;
	int e2;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);

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

void DrawingCanvas::DrawRectangle(int left, int top, int right, int bottom, Color color)
{
	DrawHLine(top, left, right, color);
	DrawHLine(bottom, left, right, color);
	DrawVLine(left, top, bottom, color);
	DrawVLine(right, top, bottom, color);
}

// Draw a rectangle in range [left, right] and [top, bottom]
void DrawingCanvas::FillRectangle(int left, int top, int right, int bottom, Color color)
{
	if(!ClipRectangle(left, top, right, bottom, m_Rect))
		return;

	int width = right - left + 1;
	int height = bottom - top + 1;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);
	int bytes = m_Format.GetBytePerPixel();

	u8* base = m_Data + top*m_Pitch + left*bytes;
	u8* cur = base;

	// Write first line
	for(int i = 0; i < width; ++i) {
		std::memcpy(cur, native_color, bytes);
		cur += bytes;
	}

	cur = base + m_Pitch;

	// Copy lines
	for(int i = 1; i < height; ++i) {
		std::memcpy(cur, base, width*bytes);
		cur += m_Pitch;
	}
}

void DrawingCanvas::DrawEllipse(int xm, int ym, int a, int b, Color color)
{
	if(a == 0 && b == 0)
		return;

	int dx = 0, dy = b;
	int a2 = a*a, b2 = b*b;
	int err = b2 - (2 * b - 1)*a2;
	int e2;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);

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

void DrawingCanvas::FillEllipse(int xm, int ym, int a, int b, Color color)
{
	if(a == 0 && b == 0)
		return;

	int dx = 0, dy = b;
	int a2 = a*a, b2 = b*b;
	int err = b2 - (2 * b - 1)*a2;
	int e2;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);

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

void DrawingCanvas::FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color)
{
	int minX = math::Min(x0, x1, x2);
	int minY = math::Min(y0, y1, y2);
	int maxX = math::Max(x0, x1, x2);
	int maxY = math::Max(y0, y1, y2);

	if(!ClipRectangle(minX, minY, maxX, maxY, m_Rect))
		return;

	u8 native_color[4];
	m_Format.A8R8G8B8ToFormat(color.ToDWORD(), native_color);

	int A01 = y0 - y1, B01 = x1 - x0;
	int A20 = y2 - y0, B20 = x0 - x2;

	if(-B20 * A01 + A20 * B01 < 0) {
		int t;
		t = x1; x1 = x2; x2 = t;
		t = y1; y1 = y2; y2 = t;

		A01 = y0 - y1; B01 = x1 - x0;
		A20 = y2 - y0; B20 = x0 - x2;
	}

	int A12 = y1 - y2, B12 = x2 - x1;

	int w0r = orient2d(x1, y1, x2, y2, minX, minY);
	int w1r = orient2d(x2, y2, x0, y0, minX, minY);
	int w2r = orient2d(x0, y0, x1, y1, minX, minY);

	for(int y = minY; y <= maxY; ++y) {
		int w0 = w0r;
		int w1 = w1r;
		int w2 = w2r;

		for(int x = minX; x <= maxX; ++x) {
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

int DrawingCanvas::GetWidth() const
{
	return m_Width;
}

int DrawingCanvas::GetHeight() const
{
	return m_Height;
}

void* DrawingCanvas::GetRaw()
{
	return m_Data;
}

ColorFormat DrawingCanvas::GetFormat() const
{
	return m_Format;
}

int DrawingCanvas::GetPitch() const
{
	return m_Pitch;
}

int DrawingCanvas::orient2d(int x0, int y0, int x1, int y1, int x2, int y2)
{
	return (x1 - x0)*(y2 - y0) - (y1 - y0)*(x2 - x0);
}

u8 DrawingCanvas::ComputeOutCode(int x, int y, const math::RectI& r)
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

bool DrawingCanvas::ClipLine(int& x0, int& y0, int& x1, int& y1, const math::RectI& r)
{
	u8 out0 = ComputeOutCode(x0, y0, r);
	u8 out1 = ComputeOutCode(x1, y1, r);

	while(true) {
		if(!(out0 | out1)) {
			return true;
		} else if(out0 & out1) {
			return false;
		} else {
			int x = 0, y = 0; // Values are never used
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

bool DrawingCanvas::ClipRectangle(int& left, int& top, int& right, int& bottom, const math::RectI&r)
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

} // namespace video
} // namespace lux

