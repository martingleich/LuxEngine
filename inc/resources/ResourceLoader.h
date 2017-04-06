#ifndef INCLUDED_RESOURCE_LOADER_H
#define INCLUDED_RESOURCE_LOADER_H
#include "resources/Resource.h"

namespace lux
{
namespace core
{

//! Interface for resource loaders
/**
Implement this interface and pass it to \ref lux::core::ResourceSystem::AddLoader "core::ResourceSystem::AddLoader" to register a new resourceloader
i.e. filetype with the engine.
*/
class ResourceLoader : public ReferenceCounted
{
public:
	//! Get the resource type contained in this file
	/**
	The filecursor is always placed at the begin of the resource file, this position doesn't have to be zero.
	This method only checks the possiblity of loading the file, i.e. checks the correct filetype.
	Maybe the rest of the file is corrupted, and later loading fails, this doesn't need to be checked here
	\param file The virtual file to check for loading
	\param requestedType The resource type requested by the loader, return value must be equal to this or empty.
		Maybe empty to indicate any possible type.
	\return The name of the resource type loaded by the file, return a empty name to indicate no loadable resource.
	*/
	virtual Name GetResourceType(io::File* file, Name requestedType = core::Name::INVALID) = 0;

	//! Loads all resources from a file
	/**
	Before loading a file the same file, was _always_ passed to \ref CanLoadResource .
	\param file The file to load resources from.
	\param dst The resource which receives the loaded data.
	\return Return false to indicate a failed load.
	*/
	virtual bool LoadResource(io::File* file, Resource* dst) = 0;

	//! Get the name of the loader
	/**
	This name is only for the user, and has no limitations.
	\return The name of the loader.
	*/
	virtual const string& GetName() const = 0;
};

}
}

#endif // #ifndef INCLUDED_RESOURCE_LOADER_H