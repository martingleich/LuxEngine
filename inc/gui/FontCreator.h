#ifndef INCLUDED_LUX_FONTCREATOR_H
#define INCLUDED_LUX_FONTCREATOR_H
#include "gui/Font.h"

namespace lux
{
namespace io
{
class File;
}
namespace gui
{

class FontCreator : public ReferenceCounted
{
public:
	virtual StrongRef<Font> CreateFontFromFile(const io::Path& path,
		const FontDescription& desc,
		const core::Array<u32>& charSet) = 0;
	virtual StrongRef<Font> CreateFontFromFile(io::File* file,
		const FontDescription& desc,
		const core::Array<u32>& charSet) = 0;
	virtual StrongRef<Font> CreateFont(const FontDescription& desc,
		const core::Array<u32>& charSet) = 0;

	virtual const core::Array<u32>& GetDefaultCharset(const core::String& name) const = 0;
};

}
}

#endif // !__INCLUDED_LUX_IFONTCREATOR