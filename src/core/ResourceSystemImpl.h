#ifndef INCLUDED_RESOURCE_SYSTEM_IMPL
#define INCLUDED_RESOURCE_SYSTEM_IMPL
#include "resources/ResourceSystem.h"
#include "resources/Resource.h"

namespace lux
{
namespace core
{

class ResourceSystemImpl : public ResourceSystem, public OriginResourceLoader
{
public:
	ResourceSystemImpl();
	~ResourceSystemImpl();
	u32 GetResourceCount(Name type) const;
	const core::String& GetResourceName(Name type, u32 id) const;
	u32 GetResourceId(Resource* resource) const;
	u32 GetResourceId(Name type, const core::String& name) const;
	void AddResource(const core::String& name, Resource* resource);
	void RemoveResource(Name type, u32 id);
	u32 FreeUnusedResources(Name type);
	StrongRef<Resource> GetResource(Name type, u32 id);
	StrongRef<Resource> GetResource(Name type, const core::String& name, bool loadIfNotFound);
	StrongRef<Resource> GetResource(Name type, io::File* file);
	StrongRef<Resource> CreateResource(Name type, const core::String& name);
	StrongRef<Resource> CreateResource(Name type, io::File* file);
	void SetCaching(Name type, bool caching);
	u32 GetResourceLoaderCount() const;
	StrongRef<ResourceLoader> GetResourceLoader(u32 id) const;
	void AddResourceWriter(ResourceWriter* writer);
	u32 GetResourceWriterCount() const;
	StrongRef<ResourceWriter> GetResourceWriter(u32 id) const;
	StrongRef<ResourceWriter> GetResourceWriter(core::Name resourceType, const core::String& ext) const;
	void WriteResource(Resource* resource,  io::File* file, const core::String& ext)  const;
	void WriteResource(Resource* resource, const io::Path& path) const;
	core::Name GetFileType(io::File* file) const;
	void AddResourceLoader(ResourceLoader* loader);
	u32 GetTypeCount() const;
	Name GetType(u32 id) const;
	void AddType(Name name);

private:
	StrongRef<Resource> CreateResource(Name type, io::File* file, const ResourceOrigin* origin);
	void LoadResource(const ResourceOrigin& origin, Resource* dst) const;

private:
	u32 GetTypeID(Name type) const;

	StrongRef<ResourceLoader> GetResourceLoader(core::Name& type, io::File* file) const;

	u32 GetResourceIdUnsafe(Name type, const core::String& name) const;
private:
	struct SelfType;
	SelfType* self;
};

}
}

#endif // #ifndef INCLUDED_RESOURCE_SYSTEM_IMPL