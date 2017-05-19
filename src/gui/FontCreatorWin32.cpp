#ifdef LUX_WINDOWS
#include "FontCreatorWin32.h"
#include "core/lxHashMap.h"
#include "io/File.h"
#include "TTFParser.h"
#include "core/lxMemory.h"
#include "core/Logger.h"
#include "StrippedWindows.h"

namespace lux
{
namespace gui
{

namespace impl
{
struct CharInfo
{
	float a;
	float b;
	float c;
};

struct Context
{
	HDC dc;

	math::dimension2du imageSize;
	HBITMAP bitmap;

	HFONT font;
	u8 fontHeight;

	core::array<u32> characters;

	core::HashMap<u32, CharInfo> charInfos;
	core::HashMap<u32, gui::CharInfo> outCharInfo;

	FontPixel* image;
	bool antialiased;
};

}

static bool GenerateDC(impl::Context* ctx)
{
	ctx->dc = CreateCompatibleDC(NULL);
	SetMapMode(ctx->dc, MM_TEXT); // Set logical units to pixel

	return (ctx->dc != NULL);
}

static void GenerateCharInfo(impl::Context* ctx)
{
	SelectObject(ctx->dc, ctx->font);
	SetTextAlign(ctx->dc, TA_TOP | TA_LEFT);

	for(auto it = ctx->characters.First(); it != ctx->characters.End(); ++it) {
		u32 ch = *it;
		SIZE charSize;
		BOOL result;
		float a, b, c;

		u16 utf16Buffer[2];
		core::CodePointToUTF16(ch, utf16Buffer);
		result = GetTextExtentPoint32W(ctx->dc, (wchar_t*)utf16Buffer, 1, &charSize);

		ctx->fontHeight = (u8)charSize.cy;
		if(result) {
			ABCFLOAT abc;
			result = GetCharABCWidthsFloatW(ctx->dc, ch, ch, &abc);
			if(result) {
				a = abc.abcfA;
				b = abc.abcfB;
				c = abc.abcfC;
			} else {
				a = c = 0;
				b = (float)charSize.cx;
			}
		} else {
			a = b = c = 0;
		}

		impl::CharInfo info;
		info.a = a;
		info.b = b;
		info.c = c;

		ctx->charInfos.Set(ch, info);
	}
}

static u32 NextPower2(u32 x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

static void CalculateImageSize(impl::Context* ctx)
{
	u32 count = 0;
	float sum_length = 0;
	for(auto it = ctx->charInfos.First(); it != ctx->charInfos.End(); ++it) {
		++count;
		sum_length += math::Max(it->b, it->a + it->b + it->c);
	}

	u32 avg_length = (u32)(sum_length / count + 2);

	u32 elems_per_line = count;
	u32 width = elems_per_line * (avg_length + 2);
	u32 height = ctx->fontHeight + 2;

	u32 p2width;
	u32 p2height;
	do {
		p2width = NextPower2(width);
		p2height = NextPower2(height);

		if(elems_per_line % 2 == 1)
			elems_per_line = elems_per_line / 2 + 1;
		else
			elems_per_line /= 2;
		width = elems_per_line * (avg_length + 2);
		height *= 2;
	} while(!(p2width == p2height || p2height * 2 == p2width));

	ctx->imageSize.width = p2width;
	ctx->imageSize.height = p2height;
}

static bool AllocBitmap(impl::Context* ctx)
{
	ctx->bitmap = CreateCompatibleBitmap(GetDC(NULL),
		ctx->imageSize.width, ctx->imageSize.height);

	return (ctx->bitmap != NULL);
}

static void FreeBitmap(impl::Context* ctx)
{
	DeleteObject(ctx->bitmap);
}

static void FreeDC(impl::Context* ctx)
{
	DeleteDC(ctx->dc);
}

static void FreeFont(impl::Context* ctx)
{
	DeleteObject(ctx->font);
}

static void GenerateFont(impl::Context* ctx)
{
	SelectObject(ctx->dc, ctx->font);
	SelectObject(ctx->dc, ctx->bitmap);

	SetTextColor(ctx->dc, RGB(0, 0, 0));
	SetTextAlign(ctx->dc, TA_TOP | TA_LEFT);
	const u32 width = ctx->imageSize.width;
	const u32 height = ctx->imageSize.height;

	RECT fullRect = {0, 0, (LONG)width, (LONG)height};
	FillRect(ctx->dc, &fullRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	SetTextAlign(ctx->dc, TA_TOP | TA_LEFT);
	SetBkMode(ctx->dc, OPAQUE);
	SetBkColor(ctx->dc, RGB(255, 255, 255));

	u32 cur_x = 0;
	u32 cur_y = 0;
	for(auto it = ctx->charInfos.First(); it != ctx->charInfos.End(); ++it) {
		const u32 c = it.key();
		const impl::CharInfo& info = it.value();

		if(cur_x + info.b + 2 > width) {
			cur_x = 0;
			cur_y += ctx->fontHeight + 2;
		}

		POINT text = {
			(LONG)(cur_x - info.a),
			(LONG)(cur_y)
		};

		u16 utf16[2];
		core::CodePointToUTF16(c, utf16);
		TextOutW(ctx->dc, text.x, text.y, (LPCWSTR)utf16, 1);

		gui::CharInfo out;
		out.A = info.a;
		out.B = info.b;
		out.C = info.c;

		out.Left = (float)(cur_x) / width;
		out.Top = (float)(cur_y) / height;
		out.Right = (float)(cur_x + info.b) / width;
		out.Bottom = (float)(cur_y + ctx->fontHeight) / height;

		ctx->outCharInfo.Set(c, out);

		auto next = it;
		++next;
		float next_a = 0;
		if(next != ctx->charInfos.End())
			next_a = next.value().a;
		cur_x += (u32)math::Max(it->b, it->b + it->c + next_a) + 2;
	}

	BITMAPFILEHEADER Header;
	ZeroMemory(&Header, sizeof(Header));
	Header.bfType = 0x4D42;
	Header.bfSize = width * height * 3;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = Header.bfSize;

	GdiFlush();

	BYTE *bits = new BYTE[bmi.bmiHeader.biSizeImage];
	GetDIBits(ctx->dc,
		ctx->bitmap,
		0,
		bmi.bmiHeader.biHeight,
		bits,
		&bmi,
		DIB_RGB_COLORS);

	ctx->image = new FontPixel[width * height];
	for(s32 y = height - 1; y >= 0; --y) {
		for(u32 x = 0; x < width; ++x) {
			u32 r = bits[(y*width + x) * 3 + 0];

			auto& px = ctx->image[(height - y - 1)*width + x];
			px.intensity = (u8)(255 - r);
		}
	}

	delete[] bits;
}

int CALLBACK EnumFontFamExProc(
	const LOGFONTW* font,
	const TEXTMETRIC* metric,
	DWORD fontType,
	LPARAM lParam)
{
	LUX_UNUSED(font);
	LUX_UNUSED(metric);
	LUX_UNUSED(fontType);
	LUX_UNUSED(lParam);

	size_t* count = (size_t*)lParam;
	*count = *count + 1;
	return 1;
}

static bool DoesFontFamilyExist(impl::Context* ctx, const string& name)
{
	LOGFONTW logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	auto utf16Buffer = core::UTF8ToUTF16(name.Data());
	size_t charCount = math::Min<size_t>(LF_FACESIZE, utf16Buffer.Size());

	memcpy(logFont.lfFaceName, utf16Buffer.Data(), charCount * 2);
	logFont.lfFaceName[charCount] = 0;
	logFont.lfCharSet = ANSI_CHARSET;
	logFont.lfPitchAndFamily = 0;

	size_t fontCounts = 0;
	EnumFontFamiliesExW(
		ctx->dc,
		&logFont,
		&EnumFontFamExProc,
		(LPARAM)&fontCounts,
		0);

	if(fontCounts != 0)
		return true;
	else
		return false;
}

static bool RegisterFileFont(io::File* file, HANDLE& outHandle, string& outFontFamily)
{
	core::mem::RawMemory data(file->GetSize());

	u32 count = file->ReadBinary(file->GetSize(), data);
	if(count != file->GetSize())
		return false;

	TTFParser ttfParser(data, data.GetSize());
	if(!ttfParser.IsValid())
		return false;

	outFontFamily = ttfParser.GetFontFamily();

	DWORD fontCount;
	outHandle = AddFontMemResourceEx(
		data,
		(DWORD)data.GetSize(),
		NULL,
		&fontCount);

	if(!outHandle)
		return false;

	return true;
}

static void UnRegisterFileFont(HANDLE handle)
{
	RemoveFontMemResourceEx(handle);
}

void* FontCreatorWin32::BeginFontCreation(io::File* file,
	const FontDescription& desc,
	const core::array<u32>& charSet)
{
	HANDLE fontResource;
	string fontFamily;
	if(!RegisterFileFont(file, fontResource, fontFamily))
		return nullptr;

	void* ctx = BeginFontCreation(true, fontFamily, desc, charSet);

	UnRegisterFileFont(fontResource);

	return ctx;
}

void* FontCreatorWin32::BeginFontCreation(const string& name,
	const FontDescription& desc,
	const core::array<u32>& charSet)
{
	return BeginFontCreation(false, name, desc, charSet);
}

static DWORD GetWin32FontWeight(EFontWeight weight)
{
	switch(weight) {
	case EFontWeight::ExtraThin:
		return FW_EXTRABOLD;
	case EFontWeight::Thin:
		return FW_LIGHT;
	case EFontWeight::Normal:
		return FW_NORMAL;
	case EFontWeight::Bolt:
		return FW_BOLD;
	case EFontWeight::ExtraBolt:
		return FW_ULTRABOLD;
	default:
		return FW_NORMAL;
	}
}

void* FontCreatorWin32::BeginFontCreation(bool isFileFont, const string& name,
	const FontDescription& desc,
	const core::array<u32>& charSet)
{
	impl::Context* ctx = new impl::Context;
	ctx->characters = charSet;
	ctx->antialiased = desc.antialiased;

	if(!GenerateDC(ctx)) {
		delete ctx;
		return nullptr;
	}

	if(isFileFont || DoesFontFamilyExist(ctx, name)) {
		ctx->font = CreateFontW(desc.size, 0,
			0, 0, GetWin32FontWeight(desc.weight),
			desc.italic != 0 ? TRUE : FALSE,
			FALSE, FALSE, DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
			ctx->antialiased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, core::StringToUTF16W(name));
		if(!ctx->font) {
			FreeDC(ctx);
			delete ctx;
			return nullptr;
		}
	} else {
		FreeDC(ctx);
		delete ctx;
		return nullptr;
	}

	GenerateCharInfo(ctx);
	CalculateImageSize(ctx);
	if(!AllocBitmap(ctx)) {
		FreeDC(ctx);
		delete ctx;
		return nullptr;
	}

	GenerateFont(ctx);

	FreeBitmap(ctx);
	FreeDC(ctx);
	FreeFont(ctx);

	return ctx;
}

bool FontCreatorWin32::GetFontImage(void* void_ctx, FontPixel*& image, math::dimension2du& imageSize)
{
	impl::Context* ctx = (impl::Context*)void_ctx;
	if(!ctx)
		return false;

	image = ctx->image;
	imageSize = ctx->imageSize;

	return true;
}

void FontCreatorWin32::GetFontInfo(void* void_ctx, u32& fontHeight)
{
	impl::Context* ctx = (impl::Context*)void_ctx;
	if(!ctx)
		return;

	fontHeight = ctx->fontHeight;
}

bool FontCreatorWin32::GetFontCharInfo(void* void_ctx, u32 character, CharInfo& outInfo)
{
	impl::Context* ctx = (impl::Context*)void_ctx;
	if(!ctx)
		return false;

	auto it = ctx->outCharInfo.Find(character);
	if(it == ctx->outCharInfo.End())
		return false;

	outInfo = *it;

	return true;
}

void FontCreatorWin32::EndFontCreation(void* void_ctx)
{
	impl::Context* ctx = (impl::Context*)void_ctx;
	if(!ctx)
		return;

	delete[] ctx->image;
	delete ctx;
}

}
}

#endif // LUX_WINDOWS
