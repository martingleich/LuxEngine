#include "FontCreatorNull.h"
#include "core/Logger.h"
#include "core/lxAlgorithm.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "video/VideoDriver.h"
#include "video/Texture.h"
#include "video/MaterialLibrary.h"

#include "gui/FontRaster.h"


namespace lux
{
namespace gui
{

FontCreatorNull::FontCreatorNull()
{
	AddDefaultCharSet("german", " AA«»íéáóúôîûâê1234567890AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZzÖöÜüÄäß²³{}[]()<>+-*,;.:!?&%§/\\'#~^°\"_´`$€@µ|=");
}

StrongRef<Font> FontCreatorNull::CreateFontFromFile(const io::Path& path,
	const FontDescription& desc,
	const core::Array<u32>& charSet)
{
	StrongRef<io::File> file = io::FileSystem::Instance()->OpenFile(path);
	return CreateFontFromFile(file, desc, charSet);
}

core::Array<u32> FontCreatorNull::CorrectCharSet(const core::Array<u32>& set)
{
	core::Array<u32> out = set;
	out.PushBack(' ');

	out.Sort();

	auto cur = out.First();
	auto first = cur + 1;
	auto end = out.End();
	while(first != end) {
		if(*first != *cur) {
			++cur;
			if(first != cur)
				*cur = *first;
		}

		++first;
	}

	out.Resize(core::IteratorDistance(out.First(), cur) + 1);

	return out;
}

StrongRef<Font> FontCreatorNull::CreateFontFromFile(io::File* file,
	const FontDescription& desc,
	const core::Array<u32>& charSet)
{
	if(!file)
		throw core::InvalidArgumentException("file", "File must not be null");

	core::Array<u32> realCharSet = CorrectCharSet(charSet);
	void* creationContext = this->BeginFontCreation(file, desc, realCharSet);
	try {
		auto out = CreateFontFromContext(creationContext, realCharSet);
		this->EndFontCreation(creationContext);
		return out;
	} catch(...) {
		this->EndFontCreation(creationContext);
		throw;
	}
}

StrongRef<Font> FontCreatorNull::CreateFont(
	const FontDescription& desc,
	const core::Array<u32>& charSet)
{
	core::Array<u32> realCharSet = CorrectCharSet(charSet);
	void* creationContext = this->BeginFontCreation(desc.name, desc, realCharSet);
	try {
		auto out = CreateFontFromContext(creationContext, realCharSet);
		this->EndFontCreation(creationContext);
		return out;
	} catch(...) {
		this->EndFontCreation(creationContext);
		throw;
	}
}

StrongRef<Font> FontCreatorNull::CreateFontFromContext(void* ctx, const core::Array<u32>& charSet)
{
	if(!ctx)
		throw core::Exception("Font creation failed");

	CharInfo info;
	u8* image;
	math::Dimension2I imageSize;
	int channelCount;
	int fontHeight;

	GetFontImage(ctx, image, imageSize, channelCount);

	FontCreationData data;
	for(auto it = charSet.First(); it != charSet.End(); ++it) {
		if(this->GetFontCharInfo(ctx, *it, info))
			data.charMap.Set(*it, info);
	}

	this->GetFontInfo(ctx, fontHeight, data.desc);
	data.charHeight = (float)fontHeight;
	data.image = image;
	data.imageSize = imageSize;
	data.baseLine = 0.0f;
	data.channelCount = channelCount;

	StrongRef<FontRaster> font = LUX_NEW(FontRaster)(core::ResourceOrigin());
	font->Init(data);
	auto settings = font->GetBaseFontSettings();
	settings.charDistance -= 2 * data.desc.borderSize;
	font->SetBaseFontSettings(settings);

	return font;
}

const core::Array<u32>& FontCreatorNull::GetDefaultCharset(const core::String& name) const
{
	auto it = m_DefaultCharSets.Find(name);
	if(it != m_DefaultCharSets.End())
		return *it;
	return *m_DefaultCharSets.First();
}

void FontCreatorNull::AddDefaultCharSet(const core::String& name, const core::String& data)
{
	core::Array<u32> a;
	for(auto it = data.First(); it != data.End(); ++it)
		a.PushBack(*it);

	m_DefaultCharSets[name] = a;
}

}
}
