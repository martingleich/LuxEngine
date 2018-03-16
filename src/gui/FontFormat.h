#ifndef INCLUDED_FONT_FORMAT_H
#define INCLUDED_FONT_FORMAT_H
#include "core/ResourceSystem.h"
#include "core/ResourceLoader.h"
#include "core/ResourceWriter.h"

namespace lux
{
namespace gui
{

class FontLoader : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
	const core::String& GetName() const;
};

class FontWriter : public core::ResourceWriter
{
public:
	bool CanWriteType(const core::String& ext, core::Name requestedType);
	void WriteResource(io::File* file, core::Resource* resource);
	const core::String& GetName() const;
};

}
}
#endif // #ifndef INCLUDED_FONT_FORMAT_H