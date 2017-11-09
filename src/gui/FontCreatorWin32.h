#ifndef INCLUDED_FONT_CREATOR_WIN32_H
#define INCLUDED_FONT_CREATOR_WIN32_H
#include "FontCreatorNull.h"

#ifdef LUX_WINDOWS
namespace lux
{
namespace gui
{

class FontCreatorWin32 : public FontCreatorNull
{
public:
	void* BeginFontCreation(io::File* file,
		const FontDescription& desc,
		const core::Array<u32>& charSet);
	void* BeginFontCreation(const core::String& name,
		const FontDescription& desc,
		const core::Array<u32>& charSet);

	void* BeginFontCreation(bool isFileFont, const core::String& name,
		const FontDescription& desc,
		const core::Array<u32>& charSet);
	bool GetFontImage(void* void_ctx, FontPixel*& image, math::Dimension2U& imageSize);
	void GetFontInfo(void*, u32& fontHeight, FontDescription& desc);
	bool GetFontCharInfo(void* void_ctx, u32 character, CharInfo& outInfo);
	void EndFontCreation(void* void_ctx);
};

}
}

#endif // LUX_WINDOWS

#endif 
