#ifndef INCLUDED_FONTCREATOR_H
#define INCLUDED_FONTCREATOR_H
#include "gui/Font.h"

namespace lux
{
namespace io
{
class File;
}
namespace gui
{

enum class EFontWeight
{
	ExtraThin,
	Thin,
	Normal,
	Bolt,
	ExtraBolt
};

struct FontDescription
{
	u32 size;
	EFontWeight weight;
	bool italic;
	bool antialiased;

	FontDescription(u32 _size,
		EFontWeight _weight = EFontWeight::Normal,
		bool _italic = false,
		bool _antialiased = true) :
		size(_size),
		weight(_weight),
		italic(_italic),
		antialiased(_antialiased)
	{}
};

class FontCreator : public ReferenceCounted
{
public:
	virtual StrongRef<Font> CreateFontFromFile(const io::path& path,
		const FontDescription& desc,
		const core::array<u32>& charSet) = 0;
	virtual StrongRef<Font> CreateFontFromFile(io::File* file,
		const FontDescription& desc,
		const core::array<u32>& charSet) = 0;
	virtual StrongRef<Font> CreateFont(const string& name,
		const FontDescription& desc,
		const core::array<u32>& charSet) = 0;

	virtual const core::array<u32>& GetDefaultCharset(const string& name) const = 0;
};

}
}

#endif // !__INCLUDED_IFONTCREATOR