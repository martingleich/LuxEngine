#ifndef INCLUDED_FONTCREATOR_NULL_H
#define INCLUDED_FONTCREATOR_NULL_H
#include "gui/FontCreator.h"
#include "gui/FontImpl.h"

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
	core::HashMap<string, core::array<u32>> m_DefaultCharSets;

public:
	FontCreatorNull();

	StrongRef<Font> CreateFont(
		const FontDescription& desc,
		const core::array<u32>& charSet);

	StrongRef<Font> CreateFontFromFile(const io::path& path,
		const FontDescription& desc,
		const core::array<u32>& charSet);
	StrongRef<Font> CreateFontFromFile(io::File* file,
		const FontDescription& desc,
		const core::array<u32>& charSet);

	const core::array<u32>& GetDefaultCharset(const string& name) const;

private:
	StrongRef<Font> CreateFontFromContext(void* ctx, const core::array<u32>& charSet);

	core::array<u32> CorrectCharSet(const core::array<u32>& set);

	void AddDefaultCharSet(const string& name, const string& data);

private:
	virtual void* BeginFontCreation(io::File* file,
		const FontDescription& desc,
		const core::array<u32>& charSet) = 0;
	virtual void* BeginFontCreation(const string& name,
		const FontDescription& desc,
		const core::array<u32>& charSet) = 0;
	virtual void GetFontInfo(void*, u32& fontHeight, FontDescription& desc) = 0;
	virtual bool GetFontImage(void*, FontPixel*& image, math::dimension2du& imageSize) = 0;
	virtual bool GetFontCharInfo(void*, u32 character, CharInfo& outInfo) = 0;
	virtual void EndFontCreation(void*) = 0;
};

}
}

#endif // !__INCLUDED_CFONTCREATORNULL