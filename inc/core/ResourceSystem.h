#ifndef INCLUDED_LUX_RESOURCESYSTEM_H
#define INCLUDED_LUX_RESOURCESYSTEM_H
#include "core/ReferenceCounted.h"
#include "core/Resource.h"
#include "core/ResourceLoader.h"
#include "core/ResourceWriter.h"
#include "core/lxName.h"
#include "io/Path.h"

namespace lux
{
namespace io
{
class File;
}
namespace core
{

//! Caching and loading for engine resources
/**
The resource system handles caching and loading of engine resources.
Here can the engine be extended with new file formats.
The resource system saves data as a map from names to resources.
*/
class ResourceSystem : public ReferenceCounted
{
public:
	//! Initialize the global resource system
	LUX_API static void Initialize();

	//! Access the global resource system
	LUX_API static ResourceSystem* Instance();

	//! Destroys the global resource system
	LUX_API static void Destroy();

	LUX_API ResourceSystem();
	LUX_API ~ResourceSystem();

	//! Remove unused resources from the system
	/**
	Unused resources are resources which are only referenced by the resource system.
	And or not otherwise used.
	\param type The name of the type, for which unused resources should be removed.
		Use the empty string to remove _all_ unused resources.
	\return The number of resources freed.
	*/
	LUX_API int FreeUnusedResources(Name type = Name::INVALID);

	//! Get a resource based on a name.
	/**
	\param type The type of the resource, can be empty to indicate any resourcetype matching name.
	\param name The name of the resource.
	\param loadIfNotFound If the resource is not found, the file <name> will be loaded.
	\return The resource, or null if the resource is not available.
	\throws FileNotFoundException
	\throws FileFormatException
	*/
	LUX_API StrongRef<core::Referable> GetResource(Name type, const io::Path& name, bool loadIfNotFound = true);

	//! Get a resources based on a file.
	/**
	First it's check if a resources with the name of the file is already loaded,
	in this case this resource is returned. Otherwise the resource is loaded from the file
	and added to the system under the name of the file.
	\param type The type of the resource.
	\param file The file from which to load the resource.
	\return The resource.
	*/
	LUX_API StrongRef<core::Referable> GetResource(Name type, io::File* file, bool loadIfNotFound = true);

	//! Enabled or disables caching for a given resource type.
	/**
	\param type The type which property to change.
	\param caching True to enable caching, false to disable it.
	*/
	LUX_API void SetCaching(Name type, bool caching);

	//! Query the number of total resource loaders.
	/**
	\return The number of resource loaders.
	*/
	LUX_API int GetResourceLoaderCount() const;

	//! Query a resource loader
	/**
	\param id The id of the resource loader must be between 0 and the number of resource loaders.
	\return A resource loader.
	*/
	LUX_API StrongRef<ResourceLoader> GetResourceLoader(int id) const;

	//! Get the resource loader matching a file.
	/**
	\param type The resourcetype to load from file, use autodetection if empty.
	\param file The file to load data from.
	\param out typeToLoad The type that will be loaded.
	\return The Resourceloader to use.
	*/
	LUX_API StrongRef<ResourceLoader> GetResourceLoader(Name type, io::File* file, Name& typeToLoad) const;

	//! Add a resource writer
	LUX_API void AddResourceWriter(ResourceWriter* writer);

	//! Query the number of total resource writers.
	/**
	\return The number of resource writers.
	*/
	LUX_API int GetResourceWriterCount() const;

	//! Query a resource writer
	/**
	\param id The id of the resource writer must be between 0 and the number of resource writer.
	\return A resource writer.
	*/
	LUX_API StrongRef<ResourceWriter> GetResourceWriter(int id) const;

	//! Get a resource writer for a type and a extension
	/**
	Returns null if the resource writer does not exist
	*/
	LUX_API StrongRef<ResourceWriter> GetResourceWriter(Name resourceType, StringView ext) const;

	//! Write a resource to a file
	/**
	\param resource The resource to write to the file.
	\param file The target file to write the resource to
	\param ext The extension(i.e. filetype) of the resource
	\throws FileFormatException
	*/
	LUX_API void WriteResource(core::Referable* resource, io::File* file, StringView ext)  const;

	//! Write a resource to a file
	/**
	\param resource The resource to write to the file.
	\param path The path of the file to write to.
	\throws FileFormatException
	*/
	LUX_API void WriteResource(core::Referable* resource, const io::Path& path) const;

	//! Get the type of resource contained in a file.
	/**
	\param file The file to check.
	\return The invalid name if there is not matching resource type.
	*/
	LUX_API Name GetFileType(io::File* file) const;

	//! Add a new resource loader to the system.
	/**
	This resource loader will be used for future resource loads.
	Resource loader which were added later in the programm, are used before
	older ones, so you can overwritte internal loaders with your own.
	\param loader The new resource loader.
	*/
	LUX_API void AddResourceLoader(ResourceLoader* loader);

	//! Get the total number of diffrent resource types.
	/**
	\return The number of resource types.
	*/
	LUX_API int GetTypeCount() const;

	//! Get the name of a resource type.
	/**
	\param id The id of the resource type, must be between 0 and the number of types.
	\return The name of the loader, or and empty string in the case of an error.
	*/
	LUX_API Name GetType(int id) const;

	//! Add a new resourcetype to the system
	/**
	Type names must be unique and not empty.
	\param name The new type.
	*/
	LUX_API void AddType(Name name);

private:
	StrongRef<core::Referable> CreateResource(int typeId, const io::Path& path);
	StrongRef<core::Referable> CreateResource(int typeId, io::File* file);
	StrongRef<core::Referable> CreateResource(int typeId, io::File* file, const ResourceOrigin& origin);

private:
	int GetTypeID(Name type) const;
	bool IsBasePath(const io::Path& path) const;

private:
	struct SelfType;
	std::unique_ptr<SelfType> self;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_RESOURCESYSTEM_H