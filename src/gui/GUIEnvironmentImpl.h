#ifndef INCLUDED_GUIENVIRONMENT_IMPL_H
#define INCLUDED_GUIENVIRONMENT_IMPL_H
#include "gui/GUIEnvironment.h"

namespace lux
{
namespace video
{
class Image;
class ImageSystem;
}
namespace gui
{
class Font;

class GUIEnvironmentImpl : public GUIEnvironment
{
public:
	GUIEnvironmentImpl(video::ImageSystem* imagSys);
	StrongRef<FontCreator> GetFontCreator();

private:
	StrongRef<FontCreator> m_FontCreator;
	StrongRef<video::ImageSystem> m_ImageSystem;
};

} 
} 

#endif