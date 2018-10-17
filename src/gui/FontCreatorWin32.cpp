#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "FontCreatorWin32.h"
#include "core/lxHashMap.h"
#include "io/File.h"
#include "TTFParser.h"
#include "core/lxMemory.h"
#include "core/Logger.h"
#include "core/SafeCast.h"
#include "platform/StrippedWindows.h"

namespace lux
{
namespace gui
{

namespace impl_fontCreatorWin32
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

	int channelCount;
	math::Dimension2I imageSize;
	HBITMAP bitmap;

	HFONT font;
	u8 fontHeight;

	core::Array<u32> characters;

	core::HashMap<u32, CharInfo> charInfos;
	core::HashMap<u32, gui::CharInfo> outCharInfo;

	u8* image;
	bool antialiased;

	int size;
	math::RectI padding;

	int borderSize;

	bool italic;
	core::String name;

	EFontWeight weight;
};

}

static bool GenerateDC(impl_fontCreatorWin32::Context* ctx)
{
	ctx->dc = CreateCompatibleDC(NULL);
	SetMapMode(ctx->dc, MM_TEXT); // Set logical units to pixel

	return (ctx->dc != NULL);
}

static void GenerateCharInfo(impl_fontCreatorWin32::Context* ctx)
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

		ctx->fontHeight = (u8)(charSize.cy + ctx->padding.left + ctx->padding.right);
		if(result) {
			ABCFLOAT abc;
			result = GetCharABCWidthsFloatW(ctx->dc, ch, ch, &abc);
			if(result) {
				a = abc.abcfA;
				b = abc.abcfB + ctx->padding.left + ctx->padding.right;
				c = abc.abcfC;
			} else {
				a = c = 0;
				b = (float)charSize.cx + ctx->padding.left + ctx->padding.right;
			}
		} else {
			a = b = c = 0;
		}

		impl_fontCreatorWin32::CharInfo info;
		info.a = a;
		info.b = b;
		info.c = c;

		ctx->charInfos.Set(ch, info);
	}
}

static int NextPower2(int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
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

static void CalculateImageSize(impl_fontCreatorWin32::Context* ctx)
{
	int count = 0;
	float sum_length = 0;
	for(auto it = ctx->charInfos.First(); it != ctx->charInfos.End(); ++it) {
		++count;
		sum_length += math::Max(it->b, it->a + it->b + it->c);
	}

	int avg_length = (int)(sum_length / count) + 2;

	int elems_per_line = count;
	int width = elems_per_line * (avg_length + 2);
	int height = ctx->fontHeight + 2;

	int p2width;
	int p2height;
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

static bool AllocBitmap(impl_fontCreatorWin32::Context* ctx)
{
	ctx->bitmap = CreateCompatibleBitmap(GetDC(NULL),
		ctx->imageSize.width, ctx->imageSize.height);

	return (ctx->bitmap != NULL);
}

static void FreeBitmap(impl_fontCreatorWin32::Context* ctx)
{
	DeleteObject(ctx->bitmap);
}

static void FreeDC(impl_fontCreatorWin32::Context* ctx)
{
	DeleteDC(ctx->dc);
}

static void FreeFont(impl_fontCreatorWin32::Context* ctx)
{
	DeleteObject(ctx->font);
}

static u8 InvertAndGammaCorrect(u8 value)
{
	// table[i] = (u8)max(0, min(255, pow((255-i)/255, 1/2.2)*255));
	static u8 table[256] = {255, 254, 254, 253, 253, 252, 252, 251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246, 246, 245, 245, 244, 244, 243, 243, 242, 242, 241, 241, 240, 240, 239, 239, 238, 238, 237, 237, 236, 236, 235, 235, 234, 234, 233, 233, 232, 232, 231, 231, 230, 230, 229, 229, 228, 228, 227, 227, 226, 226, 225, 225, 224, 224, 223, 223, 222, 222, 221, 220, 220, 219, 219, 218, 218, 217, 217, 216, 216, 215, 214, 214, 213, 213, 212, 212, 211, 210, 210, 209, 209, 208, 208, 207, 206, 206, 205, 205, 204, 203, 203, 202, 202, 201, 200, 200, 199, 199, 198, 197, 197, 196, 196, 195, 194, 194, 193, 192, 192, 191, 190, 190, 189, 189, 188, 187, 187, 186, 185, 185, 184, 183, 183, 182, 181, 181, 180, 179, 178, 178, 177, 176, 176, 175, 174, 174, 173, 172, 171, 171, 170, 169, 168, 168, 167, 166, 165, 165, 164, 163, 162, 162, 161, 160, 159, 158, 158, 157, 156, 155, 154, 153, 153, 152, 151, 150, 149, 148, 147, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 136, 135, 134, 133, 132, 131, 130, 129, 128, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 115, 114, 113, 112, 111, 109, 108, 107, 106, 104, 103, 102, 100, 99, 97, 96, 94, 93, 91, 90, 88, 87, 85, 83, 81, 80, 78, 76, 74, 72, 70, 68, 65, 63, 61, 58, 55, 52, 49, 46, 42, 38, 33, 28, 20, 0};
	return table[value];
}

static void GenerateFont(impl_fontCreatorWin32::Context* ctx)
{
	SelectObject(ctx->dc, ctx->font);
	SelectObject(ctx->dc, ctx->bitmap);

	SetTextColor(ctx->dc, RGB(0, 0, 0));
	SetTextAlign(ctx->dc, TA_TOP | TA_LEFT);
	auto width = ctx->imageSize.width;
	auto height = ctx->imageSize.height;

	RECT fullRect = {0, 0, (LONG)width, (LONG)height};
	FillRect(ctx->dc, &fullRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	SetTextAlign(ctx->dc, TA_TOP | TA_LEFT);
	SetBkMode(ctx->dc, OPAQUE);
	SetBkColor(ctx->dc, RGB(255, 255, 255));

	u32 cur_x = 0;
	u32 cur_y = 0;
	core::Array<math::RectI> charRects;
	for(auto it = ctx->charInfos.First(); it != ctx->charInfos.End(); ++it) {
		const u32 c = it.key();
		const impl_fontCreatorWin32::CharInfo& info = it.value();

		if(cur_x + info.b + 2 > width) {
			cur_x = 0;
			cur_y += ctx->fontHeight + 2;
		}

		POINT text = {
			(LONG)(cur_x - info.a + ctx->padding.left),
			(LONG)(cur_y + ctx->padding.top)
		};

		u16 utf16[2];
		core::CodePointToUTF16(c, utf16);
		TextOutW(ctx->dc, text.x, text.y, (LPCWSTR)utf16, 1);

		gui::CharInfo out;
		out.A = info.a;
		out.B = info.b;
		out.C = info.c;

		charRects.EmplaceBack(
			cur_x, cur_y,
			(int)(cur_x + info.b),
			(int)(cur_y + ctx->fontHeight));

		out.left = (float)(cur_x) / width;
		out.top = (float)(cur_y) / height;
		out.right = (float)(cur_x + info.b) / width;
		out.bottom = (float)(cur_y + ctx->fontHeight) / height;

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

	core::RawMemory bits(bmi.bmiHeader.biSizeImage);
	GetDIBits(ctx->dc,
		ctx->bitmap,
		0,
		bmi.bmiHeader.biHeight,
		bits,
		&bmi,
		DIB_RGB_COLORS);

	ctx->channelCount = ctx->borderSize > 0 ? 2 : 1;
	ctx->image = LUX_NEW_ARRAY(u8, width * height * ctx->channelCount);

	// Invert and gamma correct data, and flip upside down
	u32 inOff = 0;
	u32 baseOff = 1;
	u32 outOff = 2;
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			auto in = ((BYTE*)bits)[(y*width + x) * 3 + inOff];
			auto& out = ((BYTE*)bits)[((height - y - 1)*width + x) * 3 + baseOff];
			out = InvertAndGammaCorrect(in);
		}
	}

	// Apply dilation function
	int dilateSize = ctx->borderSize;
	int dilateCount = 1; // If changing this, the padding has to be changed too.
	if(dilateSize > 0 && dilateCount > 0) {
		for(auto& c : charRects) {
			inOff = baseOff;
			outOff = 2;
			for(int pass = 0; pass < dilateCount; ++pass) {
				for(int y = c.top; y < c.bottom; ++y) {
					for(int x = c.left; x < c.right; ++x) {
						int minX = math::Max(x - dilateSize, c.left);
						int maxX = math::Min(x + dilateSize, c.right);
						int minY = math::Max(y - dilateSize, c.top);
						int maxY = math::Min(y + dilateSize, c.bottom);
						int dilated = 0;
						for(int j = minY; j <= maxY; ++j) {
							for(int i = minX; i <= maxX; ++i) {
								auto in = ((BYTE*)bits)[(j*width + i) * 3 + inOff];
								if(in > dilated)
									dilated = in;
							}
						}
						((BYTE*)bits)[(y*width + x) * 3 + outOff] = (BYTE)dilated;
					}
				}
				if(pass == 0) {
					inOff = 2;
					outOff = 0;
				} else {
					std::swap(inOff, outOff);
				}
			}
		}
	}

	// Convert to final data
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			auto alpha = ctx->image + (y*width + x) * ctx->channelCount + 0;
			auto inner = ctx->image + (y*width + x) * ctx->channelCount + 1;

			if(ctx->borderSize) {
				int dilated = ((BYTE*)bits)[(y*width + x) * 3 + inOff];
				auto base = ((BYTE*)bits)[(y*width + x) * 3 + baseOff];
				// a+b 
				*alpha = (u8)(base + dilated - (base * dilated) / 255);
				// b/a+b
				if(*alpha != 0)
					*inner = (255 * base) / *alpha;
				else
					*inner = 255; // Any value would be okay.
			} else {
				*alpha = ((BYTE*)bits)[(y*width + x) * 3 + baseOff];
				// *inner = 255; // Don't write inner since channel not available
			}
		}
	}
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

	int* count = (int*)lParam;
	*count = *count + 1;
	return 1;
}

static bool DoesFontFamilyExist(impl_fontCreatorWin32::Context* ctx, const core::String& name)
{
	LOGFONTW logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	core::Array<u16> utf16Buffer;
	core::UTF8ToUTF16(name.Data(), name.Size(), utf16Buffer);
	int charCount = math::Min(LF_FACESIZE, utf16Buffer.Size());

	memcpy(logFont.lfFaceName, utf16Buffer.Data(), charCount * 2);
	logFont.lfFaceName[charCount] = 0;
	logFont.lfCharSet = ANSI_CHARSET;
	logFont.lfPitchAndFamily = 0;

	int fontCounts = 0;
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

static bool RegisterFileFont(io::File* file, HANDLE& outHandle, core::String& outFontFamily)
{
	core::RawMemory data(core::SafeCast<size_t>(file->GetSize()));

	file->ReadBinary(file->GetSize(), data);
	TTFParser ttfParser(data, core::SafeCast<int>(data.GetSize()));
	if(!ttfParser.IsValid())
		return false;

	outFontFamily = ttfParser.GetFontFamily();

	DWORD fontCount;
	outHandle = AddFontMemResourceEx(
		data,
		core::SafeCast<DWORD>(data.GetSize()),
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
	const core::Array<u32>& charSet)
{
	HANDLE fontResource;
	core::String fontFamily;
	if(!RegisterFileFont(file, fontResource, fontFamily))
		return nullptr;

	void* ctx = BeginFontCreation(true, fontFamily, desc, charSet);

	UnRegisterFileFont(fontResource);

	return ctx;
}

void* FontCreatorWin32::BeginFontCreation(const core::String& name,
	const FontDescription& desc,
	const core::Array<u32>& charSet)
{
	return BeginFontCreation(false, name, desc, charSet);
}

void* FontCreatorWin32::BeginFontCreation(bool isFileFont, const core::String& name,
	const FontDescription& desc,
	const core::Array<u32>& charSet)
{
	core::UniquePtr<impl_fontCreatorWin32::Context> ctx = LUX_NEW(impl_fontCreatorWin32::Context);
	ctx->characters = charSet;

	ctx->antialiased = desc.antialiased;
	ctx->italic = desc.italic;
	ctx->weight = desc.weight;
	ctx->size = desc.size;
	ctx->borderSize = desc.borderSize;
	ctx->name = name;
	ctx->padding = math::RectI(
		desc.borderSize,
		desc.borderSize,
		desc.borderSize,
		desc.borderSize);

	if(!GenerateDC(ctx))
		return nullptr;

	if(isFileFont || DoesFontFamilyExist(ctx, name)) {
		ctx->font = CreateFontW(desc.size, 0,
			0, 0, GetWin32FontWeight(ctx->weight),
			ctx->italic != 0 ? TRUE : FALSE,
			FALSE, FALSE, DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
			ctx->antialiased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, core::UTF8ToWin32String(name));
		if(!ctx->font) {
			FreeDC(ctx);
			return nullptr;
		}
	} else {
		FreeDC(ctx);
		return nullptr;
	}

	GenerateCharInfo(ctx);
	CalculateImageSize(ctx);
	if(!AllocBitmap(ctx)) {
		FreeDC(ctx);
		return nullptr;
	}

	GenerateFont(ctx);

	FreeBitmap(ctx);
	FreeDC(ctx);
	FreeFont(ctx);

	return ctx.Take();
}

bool FontCreatorWin32::GetFontImage(void* void_ctx, u8*& image, math::Dimension2I& imageSize, int& channelCount)
{
	impl_fontCreatorWin32::Context* ctx = (impl_fontCreatorWin32::Context*)void_ctx;
	if(!ctx)
		return false;

	image = ctx->image;
	imageSize = ctx->imageSize;
	channelCount = ctx->channelCount;

	return true;
}

void FontCreatorWin32::GetFontInfo(void* void_ctx, int& fontHeight, FontDescription& desc)
{
	impl_fontCreatorWin32::Context* ctx = (impl_fontCreatorWin32::Context*)void_ctx;
	if(!ctx)
		return;

	fontHeight = ctx->fontHeight;
	desc.name = ctx->name;
	desc.italic = ctx->italic;
	desc.antialiased = ctx->antialiased;
	desc.size = ctx->size;
	desc.weight = ctx->weight;
	desc.borderSize = ctx->borderSize;
}

bool FontCreatorWin32::GetFontCharInfo(void* void_ctx, u32 character, CharInfo& outInfo)
{
	impl_fontCreatorWin32::Context* ctx = (impl_fontCreatorWin32::Context*)void_ctx;
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
	impl_fontCreatorWin32::Context* ctx = (impl_fontCreatorWin32::Context*)void_ctx;
	if(!ctx)
		return;

	delete[] ctx->image;
	delete ctx;
}

}
}

#endif // LUX_WINDOWS
