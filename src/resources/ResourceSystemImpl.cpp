#include "ResourceSystemImpl.h"
#include "io/FileSystem.h"
#include "io/File.h"
#include "core/lxAlgorithm.h"
#include "core/Logger.h"

namespace lux
{
namespace core
{

namespace ResourceType
{
const Name Image = "image";
const Name ImageList = "imagelist";
const Name Texture = "texture";
const Name CubeTexture = "cubetexture";
const Name Mesh = "mesh";
const Name Font = "font";
const Name Sound = "sound";
}

struct Entry
{
	string name;
	StrongRef<Resource> resource;

	Entry()
	{
	}
	Entry(const string& n, Resource* r) : name(n), resource(r)
	{
	}

	bool operator<(const Entry& other) const
	{
		return name < other.name;
	}

	bool operator==(const Entry& other) const
	{
		return name == other.name;
	}
};

struct TypeEntry
{
	core::Name name;
	bool isCached;

	TypeEntry() :
		isCached(true)
	{
	}
	TypeEntry(core::Name n) :
		name(n),
		isCached(true)
	{
	}

	bool operator==(const TypeEntry& e) const
	{
		return name == e.name;
	}
};

struct LoaderEntry
{
	StrongRef<ResourceLoader> loader;

	LoaderEntry()
	{
	}
	LoaderEntry(ResourceLoader* l) : loader(l)
	{
	}
};

struct ResourceBlock
{
	core::array<Entry> resources;

	u32 Size() const
	{
		return (u32)resources.Size();
	}

	StrongRef<Resource> GetResource(u32 id)
	{
		if(id < resources.Size())
			return resources[id].resource;
		return nullptr;
	}

	const string& GetName(u32 id) const
	{
		if(id < resources.Size())
			return resources[id].name;
		return string::EMPTY;
	}

	u32 GetResourceId(Resource* resource) const
	{
		for(auto it = resources.First(); it != resources.End(); ++it) {
			if(it->resource == resource)
				return (u32)core::IteratorDistance(resources.First(), it);
		}

		return ResourceSystem::INVALID_ID;
	}

	u32 GetResourceId(const string& name) const
	{
		if(name.IsEmpty())
			return ResourceSystem::INVALID_ID;

		Entry entry(name, nullptr);
		auto it = core::BinarySearch(entry, resources.First(), resources.End());
		if(it == resources.End())
			return ResourceSystem::INVALID_ID;

		return (u32)core::IteratorDistance(resources.First(), it);
	}

	bool AddResource(const string& name, Resource* resource)
	{
		lxAssert(resource);
		if(!resource)
			return false;

		if(name.IsEmpty())
			return false;

		bool result = false;
		Entry entry(name, resource);
		core::array<Entry>::Iterator n;
		auto it = core::BinarySearch(entry, resources.First(), resources.End(), &n);
		if(it == resources.End()) {
			resources.Insert(entry, n);
			result = true;
		}

		return result;
	}

	bool RemoveResource(u32 id)
	{
		bool result = false;
		if(id < resources.Size()) {
			resources.Erase(core::AdvanceIterator(resources.First(), id), true);
			result = true;
		}

		return result;
	}

	u32 RemoveUnused()
	{
		const u32 oldCount = (u32)resources.Size();
		if(resources.Size() > 0) {
			auto newEnd1 = core::RemoveIf(resources.First(), resources.End(), [](const Entry& e) -> bool { return e.resource->GetReferenceCount() == 1; });
			resources.Resize(core::IteratorDistance(resources.First(), newEnd1));
		}

		const u32 newCount = (u32)resources.Size();

		return (oldCount - newCount);
	}
};

struct ResourceSystemImpl::SelfType
{
	core::array<LoaderEntry> loaders;
	core::array<TypeEntry> types;
	core::array<ResourceBlock> resources;

	StrongRef<io::FileSystem> fileSystem;
	ReferableFactory* refFactory;
};

ResourceSystemImpl::ResourceSystemImpl(io::FileSystem* fileSys, ReferableFactory* refFactory) :
	self(LUX_NEW(SelfType))
{
	self->fileSystem = fileSys;
	self->refFactory = refFactory;
}

ResourceSystemImpl::~ResourceSystemImpl()
{
	LUX_FREE(self);
}

u32 ResourceSystemImpl::GetResourceCount(Name type) const
{
	const u32 typeID = GetTypeID(type);
	if(typeID == ResourceSystem::INVALID_ID)
		return 0;
	else
		return (u32)self->resources[typeID].Size();
}

const string& ResourceSystemImpl::GetResourceName(Name type, u32 id) const
{
	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		return string::EMPTY;

	return self->resources[typeId].GetName(id);
}

u32 ResourceSystemImpl::GetResourceId(Resource* resource) const
{
	if(!resource)
		return ResourceSystem::INVALID_ID;

	const u32 typeId = GetTypeID(resource->GetReferableSubType());
	lxAssert(typeId < GetTypeCount());

	const u32 resId = self->resources[typeId].GetResourceId(resource);
	if(resId == ResourceSystem::INVALID_ID)
		return ResourceSystem::INVALID_ID;

	return resId;
}

u32 ResourceSystemImpl::GetResourceId(Name type, const string& name) const
{
	if(name.IsEmpty())
		return ResourceSystem::INVALID_ID;

	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		return ResourceSystem::INVALID_ID;

	if(self->types[typeId].isCached == false)
		return ResourceSystem::INVALID_ID;

	if(self->fileSystem->ExistFile(name)) {
		const string abs_path = self->fileSystem->GetAbsoluteFilename(name);
		return self->resources[typeId].GetResourceId(abs_path);
	} else {
		return self->resources[typeId].GetResourceId(name);
	}
}

bool ResourceSystemImpl::AddResource(const string& name, Resource* resource)
{
	if(!resource)
		return false;

	const u32 typeId = GetTypeID(resource->GetReferableSubType());
	if(typeId > GetTypeCount()) {
		log::Error("Tried to load unsupported resource type.");
		return false;
	}

	if(self->types[typeId].isCached == false)
		return true;

	return self->resources[typeId].AddResource(name, resource);
}

bool ResourceSystemImpl::RemoveResource(Name type, u32 id)
{
	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		return false;

	return self->resources[typeId].RemoveResource(id);
}

u32 ResourceSystemImpl::FreeUnusedResources(Name type)
{
	u32 count = 0;
	if(type.IsEmpty()) {
		for(auto it = self->resources.First(); it != self->resources.End(); ++it)
			count += it->RemoveUnused();
	} else {
		const u32 typeId = GetTypeID(type);
		if(typeId > GetTypeCount())
			return 0;
		count += self->resources[typeId].RemoveUnused();
	}

	return count;
}
StrongRef<Resource> ResourceSystemImpl::GetResource(Name type, u32 id)
{
	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		return nullptr;

	return self->resources[typeId].GetResource(id);
}

StrongRef<Resource> ResourceSystemImpl::GetResource(Name type, const string& name)
{
	if(name.IsEmpty())
		return nullptr;

	const u32 id = GetResourceId(type, name);
	if(id != ResourceSystem::INVALID_ID)
		return GetResource(type, id);

	StrongRef<io::File> file = self->fileSystem->OpenFile(name);
	StrongRef<Resource> out;
	if(!file)
		log::Error("Can't open file: ~s.", name);
	else {
		out = GetResource(type, file);
		if(out) {
			ResourceOrigin origin;
			origin.str = name;
			out->SetOrigin(nullptr, origin);
		}
		return out;
	}
	return nullptr;
}

StrongRef<Resource> ResourceSystemImpl::GetResource(Name type, io::File* file)
{
	if(!file)
		return nullptr;
	StrongRef<Resource> resource = CreateResource(type, file);

	if(!AddResource(file->GetName(), resource))
		resource = nullptr;

	if(resource) {
		ResourceOrigin origin;
		origin.str = file->GetName();
		resource->SetOrigin(nullptr, origin);
		resource->SetLoaded(true);
	}

	return resource;
}

StrongRef<Resource> ResourceSystemImpl::CreateResource(Name type, const string& name)
{
	if(name.IsEmpty())
		return nullptr;

	const u32 id = GetResourceId(type, name);
	if(id != ResourceSystem::INVALID_ID)
		return GetResource(type, id);

	StrongRef<io::File> file = self->fileSystem->OpenFile(name);
	if(!file)
		log::Error("Can't open file: ~s.", name);
	return CreateResource(type, file);
}

StrongRef<Resource> ResourceSystemImpl::CreateResource(Name type, io::File* file)
{
	// Get loader and correct resource type from file
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file);
	if(!loader) {
		log::Error("Resource format is not supported: ~s.", file->GetName());
		return nullptr;
	}

	// TODO: Use origin and originloader to load data
	StrongRef<Resource> resource = self->refFactory->Create(ReferableType::Resource, type);
	if(!resource)
		return nullptr;

	const u32 oldCursor = file->GetCursor();
	const bool result = loader->LoadResource(file, resource);

	if(!result) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		log::Error("File not supported or corrupted: ~s.", file->GetName());
		return nullptr;
	}

	return resource;
}

void ResourceSystemImpl::SetCaching(Name type, bool caching)
{
	u32 typeId = GetTypeID(type);
	if(typeId >= self->types.Size())
		return;

	if(self->types[typeId].isCached == true && caching == false)
		self->resources[typeId].resources.Clear();

	self->types[typeId].isCached = caching;
}

u32 ResourceSystemImpl::GetResourceLoaderCount() const
{
	return (u32)self->loaders.Size();
}

StrongRef<ResourceLoader> ResourceSystemImpl::GetResourceLoader(u32 id) const
{
	if(id < GetResourceLoaderCount())
		return self->loaders[id].loader;
	else
		return nullptr;
}

core::Name ResourceSystemImpl::GetFileType(io::File* file) const
{
	core::Name type;
	if(GetResourceLoader(type, file))
		return type;
	else
		return core::Name::INVALID;

}

bool ResourceSystemImpl::AddResourceLoader(ResourceLoader* loader)
{
	if(!loader)
		return false;

	log::Debug("Registered resource loader: ~s.", loader->GetName());
	self->loaders.PushBack(loader);

	return true;
}

u32 ResourceSystemImpl::GetTypeCount() const
{
	return (u32)self->types.Size();
}

Name ResourceSystemImpl::GetType(u32 id) const
{
	if(id < GetTypeCount())
		return self->types[id].name;
	else
		return core::Name::INVALID;
}

bool ResourceSystemImpl::AddType(Name name)
{
	TypeEntry entry(name);
	auto it = core::LinearSearch(entry, self->types.First(), self->types.End());
	if(it == self->types.End()) {
		self->types.PushBack(entry);
		self->resources.PushBack(ResourceBlock());
		log::Debug("New resource type \"~s\".", name);
		return true;
	} else {
		log::Error("Type \"~s\" already exists.", name);
		return false;
	}
}

u32 ResourceSystemImpl::GetTypeID(Name type) const
{
	TypeEntry entry(type);
	auto it = core::LinearSearch(entry, self->types.First(), self->types.End());
	if(it != self->types.End())
		return (u32)core::IteratorDistance(self->types.First(), it);
	else
		return ResourceSystem::INVALID_ID;
}

StrongRef<ResourceLoader> ResourceSystemImpl::GetResourceLoader(core::Name& type, io::File* file) const
{
	const u32 fileCursor = file->GetCursor();
	StrongRef<ResourceLoader> result;
	for(auto it = self->loaders.Last(); it != self->loaders.Begin(); --it) {
		core::Name fileType = it->loader->GetResourceType(file, type);
		if(fileType != core::Name::INVALID) {
			if(type == core::Name::INVALID || type == fileType) {
				type = fileType;
				result = it->loader;
			}
		}

		if(!file->Seek(fileCursor, io::ESeekOrigin::Start)) {
			log::Error("Can't seek file for resource loader search.");
			result = nullptr;
			break;
		}

		if(result != nullptr)
			break;
	}

	return result;
}


StrongRef<ReferableFactory> ResourceSystemImpl::GetReferableFactory()
{
	return self->refFactory;
}

StrongRef<io::FileSystem> ResourceSystemImpl::GetFileSystem()
{
	return self->fileSystem;
}

}
}