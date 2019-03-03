#ifndef INCLUDED_LUX_FONT_CREATOR_WIN32_H
#define INCLUDED_LUX_FONT_CREATOR_WIN32_H
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
	void GetFontImage(void* void_ctx, u8*& image, math::Dimension2I& imageSize, int& channelCount);
	void GetFontInfo(void*, int& fontHeight, FontDescription& desc);
	core::Optional<CharInfo> GetFontCharInfo(void* void_ctx, u32 character);
	void EndFontCreation(void* void_ctx);
};

}
}

#endif // LUX_WINDOWS

#endif 
