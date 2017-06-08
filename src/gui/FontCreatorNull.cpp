#include "FontCreatorNull.h"
#include "video/VideoDriver.h"
#include "video/MaterialLibrary.h"
#include "video/Texture.h"
#include "gui/FontImpl.h"
#include "io/FileSystem.h"
#include "core/Logger.h"
#include "io/File.h"
#include "core/lxAlgorithm.h"
#include "core/lxSort.h"
#include "video/AlphaSettings.h"
#include "video/MaterialRenderer.h"

namespace lux
{
namespace gui
{

FontCreatorNull::FontCreatorNull(video::MaterialLibrary* matLib,
	video::VideoDriver* driver,
	video::MaterialRenderer* defaultFontMaterial) :
	m_Driver(driver),
	m_DefaultFontMaterial(defaultFontMaterial)
{
	if(!m_DefaultFontMaterial)
		m_DefaultFontMaterial = matLib->GetMaterialRenderer("font");

	AddDefaultCharSet("german", " AA«»íéáóúôîûâê1234567890AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZzÖöÜüÄäß²³{}[]()<>+-*,;.:!?&%§/\\'#~^°\"_´`$€@µ|=");
}

StrongRef<Font> FontCreatorNull::CreateFontFromFile(const io::path& path,
	const FontDescription& desc,
	const core::array<u32>& charSet)
{
	StrongRef<io::File> file = io::FileSystem::Instance()->OpenFile(path);
	return CreateFontFromFile(file, desc, charSet);
}

core::array<u32> FontCreatorNull::CorrectCharSet(const core::array<u32>& set)
{
	core::array<u32> out = set;
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
	const core::array<u32>& charSet)
{
	if(!file)
		throw core::InvalidArgumentException("file", "File must not be null");

	core::array<u32> realCharSet = CorrectCharSet(charSet);
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

StrongRef<Font> FontCreatorNull::CreateFont(const string& name,
	const FontDescription& desc,
	const core::array<u32>& charSet)
{
	core::array<u32> realCharSet = CorrectCharSet(charSet);
	void* creationContext = this->BeginFontCreation(name, desc, realCharSet);
	try {
		auto out = CreateFontFromContext(creationContext, realCharSet);
		this->EndFontCreation(creationContext);
		return out;
	} catch(...) {
		this->EndFontCreation(creationContext);
		throw;
	}
}

StrongRef<Font> FontCreatorNull::CreateFontFromContext(void* ctx, const core::array<u32>& charSet)
{
	if(!ctx)
		throw core::Exception("Font creation failed");

	CharInfo info;
	FontPixel* image;
	math::dimension2du imageSize;
	u32 fontHeight;

	GetFontImage(ctx, image, imageSize);

	FontCreationData data;
	for(auto it = charSet.First(); it != charSet.End(); ++it) {
		if(this->GetFontCharInfo(ctx, *it, info))
			data.charMap.Set(*it, info);
	}

	this->GetFontInfo(ctx, fontHeight);

	data.charHeight = (float)fontHeight;
	data.material = m_DefaultFontMaterial->CreateMaterial();
	video::AlphaBlendSettings alpha(video::EBlendFactor::SrcAlpha, video::EBlendFactor::OneMinusSrcAlpha, video::EBlendOperator::Add);
	data.material->Param("blendFunc") = alpha.Pack();
	data.image = image;
	data.imageWidth = imageSize.width;
	data.imageHeight = imageSize.height;
	data.charDistance = 0.0f;
	data.wordDistance = 1.0f;
	data.lineDistance = 1.0f;
	data.scale = 1.0f;
	data.baseLine = 0.0f;

	StrongRef<FontImpl> font = LUX_NEW(FontImpl);
	font->Init(m_Driver, data);

	return font;
}

const core::array<u32>& FontCreatorNull::GetDefaultCharset(const string& name) const
{
	auto it = m_DefaultCharSets.Find(name);
	if(it != m_DefaultCharSets.End())
		return *it;
	return *m_DefaultCharSets.First();
}

void FontCreatorNull::AddDefaultCharSet(const string& name, const string& data)
{
	core::array<u32> a;
	for(auto it = data.First(); it != data.End(); ++it)
		a.PushBack(*it);

	m_DefaultCharSets[name] = a;
}

}
}
