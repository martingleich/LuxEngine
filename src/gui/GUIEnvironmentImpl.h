#ifndef INCLUDED_GUIENVIRONMENT_IMPL_H
#define INCLUDED_GUIENVIRONMENT_IMPL_H
#include "gui/GUIEnvironment.h"

namespace lux
{
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
		video::ImageSystem* ImagSys,
		video::VideoDriver* Driver,
		video::MaterialLibrary* matLib);

	StrongRef<FontCreator> GetFontCreator();
private:
	StrongRef<FontCreator> m_FontCreator;

	StrongRef<video::MaterialLibrary> m_MatLibrary;
	StrongRef<video::ImageSystem> m_ImageSystem;
	StrongRef<video::VideoDriver> m_VideoDriver;
};

} 
} 

#endif