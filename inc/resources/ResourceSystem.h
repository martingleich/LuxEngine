#ifndef INCLUDED_RESOURCESYSTEM
#define INCLUDED_RESOURCESYSTEM
#include "core/ReferableFactory.h"
#include "ResourceLoader.h"

namespace lux
{
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
	//! Returned by the resource system, if asked for an invalid id.
	static const u32 INVALID_ID = 0xFFFFFFFF;

public:
	virtual ~ResourceSystem() {}

	//! Query the number of loaded resources of a given type.
	/**
	\param type The resource type for which to query the number of resources.
	\return The number of loaded resources
	*/
	virtual u32 GetResourceCount(Name type) const = 0;

	//! Query the name of a resource
	/**
	\param type The type of the resource.
	\param id The index of the resource.
		The index must be a number between 0 and the number of loaded resources
		of the type.
	\return The name of the resource.
	*/
	virtual const string& GetResourceName(Name type, u32 id) const = 0;

	//! Query the index of a resource
	/**
	Remark: The id is only valid until resources are added or removed from the system.
	It's best to use an id immeditly after getting it.
	\param resource The resource to query the name from.
	\return The index of the resource, or INVALID_ID if an error occured.
	*/
	virtual u32 GetResourceId(Resource* resource) const = 0;

	//! Query the index of a resource.
	/**
	\param type The type of the resource to query.
	\param name The name of the resource to query.
	\return The index of the resource, or INVALID_ID if an error occured.
	*/
	virtual u32 GetResourceId(Name type, const string& name) const = 0;

	//! Add a resource to the system
	/**
	The name of resource must be unique for each type.
	\param name The name of the resource.
	\param resource The resource to add to the system.
	\return True, if the resource was added to the system, otherwise false.
	*/
	virtual bool AddResource(const string& name, Resource* resource) = 0;

	//! Remove a resource from the cache
	/**
	\param type The type of the resource to remove.
	\param id The id of the resource to remove.
	\return True, if the resource was removed, otherwise false.
	*/
	virtual bool RemoveResource(Name type, u32 id) = 0;

	//! Remove unused resources from the system
	/**
	Unused resources are resources which are only referenced by the resource system.
	And or not otherwise used.
	\param type The name of the type, for which unused resources should be removed.
		Use the empty string to remove _all_ unused resources.
	\return The number of resources freed.
	*/
	virtual u32 FreeUnusedResources(Name type = Name::INVALID) = 0;

	//! Get a resource based on a id.
	/**
	\param type The type of the resource.
	\param id The index of the resource.
	\return A resource or null in case of an error.
	*/
	virtual StrongRef<Resource> GetResource(Name type, u32 id) = 0;

	//! Get a resource based on a name.
	/**
	If no resource with the given name isn't found, it's tried to load a file with
	the given name. <br>
	\param type The type of the resource, can be empty to indicate any resourcetype matching name.
	\param name The name of the resource.
	\return The resource of null in case of an error.
	*/
	virtual StrongRef<Resource> GetResource(Name type, const string& name) = 0;

	//! Get a resources based on a file.
	/**
	First it's check if a resources with the name of the file is already loaded,
	in this case this resource is returned. Otherwise the resource is loaded from the file
	and added to the system under the name of the file.
	\param type The type of the resource.
	\param file The file from which to load the resource.
	\return The resource of null in case of an error.
	*/
	virtual StrongRef<Resource> GetResource(Name type, io::File* file) = 0;

	//! Create a unmanaged resource base on a file
	/**
	\param type The type of the resource.
	\param file The file from which to load the resource.
	\return The resource of null in case of an error.
	*/
	virtual StrongRef<Resource> CreateResource(Name type, io::File* file) = 0;

	//! Create a unmanaged resources based on a name.
	/**
	If no resource with the given name isn't found, it's tried to load a file with
	the given name. <br>
	\param type The type of the resource, can be empty to indicate any resourcetype matching name.
	\param name The name of the resource.
	\return The resource of null in case of an error.
	*/
	virtual StrongRef<Resource> CreateResource(Name type, const string& name) = 0;

	//! Enabled or disables caching for a given resource type.
	/**
	\param type The type which property to change.
	\param caching True to enable caching, false to disable it.
	*/
	virtual void SetCaching(Name type, bool caching) = 0;

	//! Query the number of total resource loaders.
	/**
	\return The number of resource loaders.
	*/
	virtual u32 GetResourceLoaderCount() const = 0;

	//! Query a resource loader
	/**
	\param id The id of the resource loader must be between 0 and the number of resource loaders.
	\return A resource loader, or null in case of an error.
	*/
	virtual StrongRef<ResourceLoader> GetResourceLoader(u32 id) const = 0;

	virtual core::Name GetFileType(io::File* file) const = 0;

	//! Add a new resource loader to the system.
	/**
	This resource loader will be used for future resource loads.
	Resource loader which were added later in the programm, are used before
	older ones, so you can overwritte internal loaders with your own.
	\param loader The new resource loader.
	\return True if the loaders was added otherwise false.
	*/
	virtual bool AddResourceLoader(ResourceLoader* loader) = 0;

	//! Get the total number of diffrent resource types.
	/**
	\return The number of resource types.
	*/
	virtual u32 GetTypeCount() const = 0;

	//! Get the name of a resource type.
	/**
	\param id The id of the resource type, must be between 0 and the number of types.
	\return The name of the loader, or and empty string in the case of an error.
	*/
	virtual Name GetType(u32 id) const = 0;

	//! Add a new resourcetype to the system
	/**
	Type names must be unique and not empty.
	\param name The new type.
	\return True, if the type was added, otherwise false.
	*/
	virtual bool AddType(Name name) = 0;

	virtual StrongRef<ReferableFactory> GetReferableFactor() = 0;
};

}
}

#endif // #ifndef INCLUDED_RESOURCESYSTEM