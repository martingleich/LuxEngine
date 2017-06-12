#ifndef INCLUDED_RESOURCE_WRITER_H
#define INCLUDED_RESOURCE_WRITER_H
#include "resources/Resource.h"

namespace lux
{
namespace core
{

//! Interface for resource writers
/**
Implement this interface and pass it to \ref lux::core::ResourceSystem::AddWriter "core::ResourceSystem::AddWriter" to register a new resource writer
i.e. filetype with the engine, and write resources to files.
*/
class ResourceWriter : public ReferenceCounted
{
public:
	//! Can a resource of a given type, be written into some file
	/**
	\param ext The extension for the written file (jpg, bmp, dds, obj, etc.)
	\param requestedType The type of the resource, the user wishes to write to file.
	\return Can the writer, write the given type
	*/
	virtual bool CanWriteType(const string& ext, Name requestedType) = 0;

	//! Write a resource to file
	virtual void WriteResource(io::File* file, Resource* resource) = 0;

	//! Get the name of the writer
	/**
	This name is only for the user, and has no limitations.
	\return The name of the writer.
	*/
	virtual const string& GetName() const = 0;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_RESOURCE_LOADER_H
