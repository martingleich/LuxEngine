#ifndef INCLUDED_FONT_LOADER_H
#define INCLUDED_FONT_LOADER_H
#include "resources/ResourceSystem.h"
#include "resources/ResourceLoader.h"

namespace lux
{
namespace video
{
class ImageSystem;
}
namespace gui
{
class Font;

class FontLoader : public core::ResourceLoader
{
public:
	FontLoader()
	{}

	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
	const string& GetName() const;

private:
	void LoadFontFromFile(io::File* file, core::Resource* dst);
};

}
}
#endif // #ifndef INCLUDED_FONT_LOADER_H