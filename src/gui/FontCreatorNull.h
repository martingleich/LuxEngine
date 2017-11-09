#ifndef INCLUDED_FONTCREATOR_NULL_H
#define INCLUDED_FONTCREATOR_NULL_H
#include "gui/FontCreator.h"
#include "gui/FontRaster.h"

namespace lux
{
namespace video
{
class MaterialLibrary;
class MaterialRenderer;
}
namespace gui
{

class FontCreatorNull : public FontCreator
{
private:
	core::HashMap<core::String, core::Array<u32>> m_DefaultCharSets;

public:
	FontCreatorNull();

	StrongRef<Font> CreateFont(
		const FontDescription& desc,
		const core::Array<u32>& charSet);

	StrongRef<Font> CreateFontFromFile(const io::Path& path,
		const FontDescription& desc,
		const core::Array<u32>& charSet);
	StrongRef<Font> CreateFontFromFile(io::File* file,
		const FontDescription& desc,
		const core::Array<u32>& charSet);

	const core::Array<u32>& GetDefaultCharset(const core::String& name) const;

private:
	StrongRef<Font> CreateFontFromContext(void* ctx, const core::Array<u32>& charSet);

	core::Array<u32> CorrectCharSet(const core::Array<u32>& set);

	void AddDefaultCharSet(const core::String& name, const core::String& data);

private:
	virtual void* BeginFontCreation(io::File* file,
		const FontDescription& desc,
		const core::Array<u32>& charSet) = 0;
	virtual void* BeginFontCreation(const core::String& name,
		const FontDescription& desc,
		const core::Array<u32>& charSet) = 0;
	virtual void GetFontInfo(void*, u32& fontHeight, FontDescription& desc) = 0;
	virtual bool GetFontImage(void*, FontPixel*& image, math::Dimension2U& imageSize) = 0;
	virtual bool GetFontCharInfo(void*, u32 character, CharInfo& outInfo) = 0;
	virtual void EndFontCreation(void*) = 0;
};

}
}

#endif // !__INCLUDED_CFONTCREATORNULL