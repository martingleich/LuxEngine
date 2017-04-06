#ifndef INCLUDED_FONT_LOADER_H
#define INCLUDED_FONT_LOADER_H
#include "resources/ResourceSystem.h"

namespace lux
{
namespace video
{
class ImageSystem;
class VideoDriver;
class MaterialLibrary;
}
namespace gui
{
class Font;

class FontLoader : public core::ResourceLoader
{
public:
	FontLoader(video::ImageSystem* imgSys,
		video::VideoDriver* driver,
		video::MaterialLibrary* matLib) :
		m_MaterialLibrary(matLib),
		m_ImageSystem(imgSys),
		m_Driver(driver)
	{}

	core::Name GetResourceType(io::File* file, core::Name requestedType);
	bool LoadResource(io::File* file, core::Resource* dst);
	const string& GetName() const;

private:
	bool LoadFontFromFile(io::File* file, core::Resource* dst);

private:
	WeakRef<video::MaterialLibrary> m_MaterialLibrary;
	WeakRef<video::ImageSystem> m_ImageSystem;
	WeakRef<video::VideoDriver> m_Driver;
};

}
}
#endif // #ifndef INCLUDED_FONT_LOADER_H