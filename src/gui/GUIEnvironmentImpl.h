#ifndef INCLUDED_GUIENVIRONMENT_IMPL_H
#define INCLUDED_GUIENVIRONMENT_IMPL_H
#include "gui/GUIEnvironment.h"


namespace lux
{
namespace core
{
class ResourceSystem;
}
namespace video
{
class Image;
class ImageSystem;
class VideoDriver;
class MaterialLibrary;
}
namespace gui
{
class Font;

class GUIEnvironmentImpl : public GUIEnvironment
{
public:
	GUIEnvironmentImpl(
		core::ResourceSystem* resSys,
		video::ImageSystem* ImagSys,
		video::VideoDriver* Driver,
		video::MaterialLibrary* matLib,
		io::FileSystem* fileSys);

	~GUIEnvironmentImpl();
	StrongRef<Font> GetFont(const io::path& name);
	StrongRef<FontCreator> GetFontCreator();

private:
	StrongRef<FontCreator> m_FontCreator;

	StrongRef<core::ResourceSystem> m_ResSys;

	StrongRef<video::MaterialLibrary> m_MatLibrary;
	StrongRef<video::ImageSystem> m_ImageSystem;
	StrongRef<video::VideoDriver> m_VideoDriver;
};


} 

} 


#endif