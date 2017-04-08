#ifndef INCLUDED_FONT_CREATOR_WIN32_H
#define INCLUDED_FONT_CREATOR_WIN32_H
#include "FontCreatorNull.h"

namespace lux
{
namespace gui
{

class FontCreatorWin32 : public FontCreatorNull
{
public:
	FontCreatorWin32(video::MaterialLibrary* matLib,
		io::FileSystem* fileSys,
		video::VideoDriver* driver,
		video::MaterialRenderer* defaultFontMaterial) :
		FontCreatorNull(matLib, fileSys, driver, defaultFontMaterial)
	{
	}

	void* BeginFontCreation(io::File* file,
		const FontDescription& desc,
		const core::array<u32>& charSet);
	void* BeginFontCreation(const string& name,
		const FontDescription& desc,
		const core::array<u32>& charSet);

	void* BeginFontCreation(bool isFileFont, const string& name,
		const FontDescription& desc,
		const core::array<u32>& charSet);
	bool GetFontImage(void* void_ctx, FontPixel*& image, math::dimension2du& imageSize);
	void GetFontInfo(void*, u32& fontHeight);
	bool GetFontCharInfo(void* void_ctx, u32 character, CharInfo& outInfo);
	void EndFontCreation(void* void_ctx);
};

}
}
#endif // #ifndef INCLUDED_FONT_CREATOR_WIN32_H