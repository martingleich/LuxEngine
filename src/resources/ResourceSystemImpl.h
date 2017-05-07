#ifndef INCLUDED_RESOURCE_SYSTEM_IMPL
#define INCLUDED_RESOURCE_SYSTEM_IMPL
#include "resources/ResourceSystem.h"

namespace lux
{

namespace io
{
class FileSystem;
}

namespace core
{

class ResourceSystemImpl : public ResourceSystem
{
public:
	ResourceSystemImpl(io::FileSystem* fileSys, ReferableFactory* refFactory);
	~ResourceSystemImpl();
	u32 GetResourceCount(Name type) const;
	const string& GetResourceName(Name type, u32 id) const;
	u32 GetResourceId(Resource* resource) const;
	u32 GetResourceId(Name type, const string& name) const;
	void AddResource(const string& name, Resource* resource);
	void RemoveResource(Name type, u32 id);
	u32 FreeUnusedResources(Name type);
	StrongRef<Resource> GetResource(Name type, u32 id);
	StrongRef<Resource> GetResource(Name type, const string& name);
	StrongRef<Resource> GetResource(Name type, io::File* file);
	StrongRef<Resource> CreateResource(Name type, const string& name);
	StrongRef<Resource> CreateResource(Name type, io::File* file);
	void SetCaching(Name type, bool caching);
	u32 GetResourceLoaderCount() const;
	StrongRef<ResourceLoader> GetResourceLoader(u32 id) const;
	core::Name GetFileType(io::File* file) const;
	void AddResourceLoader(ResourceLoader* loader);
	u32 GetTypeCount() const;
	Name GetType(u32 id) const;
	void AddType(Name name);

	StrongRef<ReferableFactory> GetReferableFactory();
	StrongRef<io::FileSystem> GetFileSystem();
private:
	u32 GetTypeID(Name type) const;

	StrongRef<ResourceLoader> GetResourceLoader(core::Name& type, io::File* file) const;

	u32 GetResourceIdUnsafe(Name type, const string& name) const;
private:
	struct SelfType;
	SelfType* self;
};

}
}

#endif // #ifndef INCLUDED_RESOURCE_SYSTEM_IMPL