#include "core/ResourceSystem.h"
#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"
#include "core/Logger.h"

#include "io/FileSystem.h"
#include "io/File.h"

namespace lux
{
namespace core
{

namespace ResourceType
{
const Name Image("lux.resource.Image");
const Name ImageList("lux.resouce.Imagelist");
const Name Texture("lux.resource.Texture");
const Name CubeTexture("lux.resource.CubeTexture");
const Name Mesh("lux.resource.Mesh");
const Name Font("lux.resource.Font");
const Name Sound("lux.resource.Sound");
}

static u32 INVALID_ID = 0xFFFFFFFF;

struct Entry
{
	String name;
	StrongRef<Resource> resource;

	Entry()
	{
	}
	Entry(const String& n, Resource* r) : name(n), resource(r)
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
	Name name;
	bool isCached;

	TypeEntry() :
		isCached(true)
	{
	}
	TypeEntry(Name n) :
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
	Array<Entry> resources;

	u32 Size() const
	{
		return (u32)resources.Size();
	}

	StrongRef<Resource> GetResource(u32 id)
	{
		return resources.At(id).resource;
	}

	const String& GetName(u32 id) const
	{
		return resources.At(id).name;
	}

	u32 GetResourceId(Resource* resource) const
	{
		for(auto it = resources.First(); it != resources.End(); ++it) {
			if(it->resource == resource)
				return (u32)IteratorDistance(resources.First(), it);
		}

		return INVALID_ID;
	}

	u32 GetResourceId(const String& name) const
	{
		if(name.IsEmpty())
			return INVALID_ID;

		Entry entry(name, nullptr);
		auto it = BinarySearch(entry, resources);
		if(it == resources.End())
			return INVALID_ID;

		return (u32)IteratorDistance(resources.First(), it);
	}

	void AddResource(const String& name, Resource* resource)
	{
		lxAssert(resource);
		if(!resource)
			throw InvalidArgumentException("resource", "Must not be null");

		if(name.IsEmpty())
			throw InvalidArgumentException("name", "Must not be empty");

		Entry entry(name, resource);
		Array<Entry>::Iterator n;
		auto it = BinarySearch(entry, resources, &n);
		if(it != resources.End())
			throw Exception("Resource already exists");

		resources.Insert(entry, n);
	}

	void RemoveResource(u32 id)
	{
		if(id >= resources.Size())
			throw OutOfRangeException();

		resources.Erase(AdvanceIterator(resources.First(), id), true);
	}

	u32 RemoveUnused()
	{
		const u32 oldCount = (u32)resources.Size();
		if(resources.Size() > 0) {
			auto newEnd1 = RemoveIf(resources, [](const Entry& e) -> bool { return e.resource->GetReferenceCount() == 1; });
			resources.Resize(IteratorDistance(resources.First(), newEnd1));
		}

		const u32 newCount = (u32)resources.Size();

		return (oldCount - newCount);
	}
};

struct ResourceSystem::SelfType
{
	Array<LoaderEntry> loaders;
	Array<StrongRef<ResourceWriter>> writers;

	Array<TypeEntry> types;
	Array<ResourceBlock> resources;

	io::FileSystem* fileSystem;
	ReferableFactory* refFactory;
};

static StrongRef<ResourceSystem> g_ResourceSystem;

void ResourceSystem::Initialize()
{
	if(!g_ResourceSystem)
		g_ResourceSystem = LUX_NEW(ResourceSystem);
}

ResourceSystem* ResourceSystem::Instance()
{
	return g_ResourceSystem;
}

void ResourceSystem::Destroy()
{
	g_ResourceSystem.Reset();
}

ResourceSystem::ResourceSystem() :
	self(LUX_NEW(SelfType))
{
	self->fileSystem = io::FileSystem::Instance();
	self->refFactory = ReferableFactory::Instance();
}

ResourceSystem::~ResourceSystem()
{
	LUX_FREE(self);
}

u32 ResourceSystem::GetResourceCount(Name type) const
{
	const u32 typeID = GetTypeID(type);
	if(typeID == INVALID_ID)
		throw InvalidArgumentException("type", "type does not exist");

	return (u32)self->resources[typeID].Size();
}

const String& ResourceSystem::GetResourceName(Name type, u32 id) const
{
	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		throw Exception("Type does not exist");

	return self->resources[typeId].GetName(id);
}

u32 ResourceSystem::GetResourceId(Resource* resource) const
{
	if(!resource)
		throw InvalidArgumentException("resource", "Must not be null");

	const u32 typeId = GetTypeID(resource->GetReferableType());

	const u32 resId = self->resources[typeId].GetResourceId(resource);
	if(resId == INVALID_ID)
		throw Exception("Resource does not exist");

	return resId;
}

u32 ResourceSystem::GetResourceId(Name type, const String& name) const
{
	u32 id = GetResourceIdUnsafe(type, name);
	if(id == INVALID_ID)
		throw ObjectNotFoundException(name.Data());

	return id;
}

u32 ResourceSystem::GetResourceIdUnsafe(Name type, const String& name) const
{
	if(name.IsEmpty())
		throw InvalidArgumentException("name", "Must not be empty");

	const u32 typeId = GetTypeID(type);

	if(self->types[typeId].isCached == false)
		return INVALID_ID;

	if(self->fileSystem->ExistFile(name)) {
		const String abs_path = self->fileSystem->GetAbsoluteFilename(name);
		return self->resources[typeId].GetResourceId(abs_path);
	} else {
		return self->resources[typeId].GetResourceId(name);
	}
}

void ResourceSystem::AddResource(const String& name, Resource* resource)
{
	if(!resource)
		throw InvalidArgumentException("resource", "Must not be null");

	const u32 typeId = GetTypeID(resource->GetReferableType());
	if(self->types[typeId].isCached == false)
		return;

	self->resources[typeId].AddResource(name, resource);
}

void ResourceSystem::RemoveResource(Name type, u32 id)
{
	const u32 typeId = GetTypeID(type);

	self->resources[typeId].RemoveResource(id);
}

u32 ResourceSystem::FreeUnusedResources(Name type)
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
StrongRef<Resource> ResourceSystem::GetResource(Name type, u32 id)
{
	const u32 typeId = GetTypeID(type);

	return self->resources[typeId].GetResource(id);
}

StrongRef<Resource> ResourceSystem::GetResource(Name type, const String& name, bool loadIfNotFound)
{
	if(name.IsEmpty())
		return nullptr;

	const u32 id = GetResourceIdUnsafe(type, name);
	if(id != INVALID_ID)
		return GetResource(type, id);

	StrongRef<Resource> resource;
	if(loadIfNotFound) {
		auto file = self->fileSystem->OpenFile(name);

		ResourceOrigin origin(this, name);
		resource = CreateResource(type, file, &origin);
		AddResource(file->GetName(), resource);
	}

	return resource;
}

StrongRef<Resource> ResourceSystem::GetResource(Name type, io::File* file)
{
	if(!file)
		throw InvalidArgumentException("file", "Must not be null");

	StrongRef<Resource> resource = CreateResource(type, file);
	AddResource(file->GetName(), resource);

	return resource;
}

StrongRef<Resource> ResourceSystem::CreateResource(Name type, const String& name)
{
	if(name.IsEmpty())
		throw InvalidArgumentException("name", "Name may not be empty");

	const u32 id = GetResourceIdUnsafe(type, name);
	if(id != INVALID_ID)
		return GetResource(type, id);

	StrongRef<io::File> file = self->fileSystem->OpenFile(name);

	ResourceOrigin origin(this, name);
	return CreateResource(type, file, &origin);
}

StrongRef<Resource> ResourceSystem::CreateResource(Name type, io::File* file)
{
	return CreateResource(type, file, nullptr);
}

void ResourceSystem::SetCaching(Name type, bool caching)
{
	u32 typeId = GetTypeID(type);
	if(typeId >= self->types.Size())
		return;

	if(self->types[typeId].isCached == true && caching == false)
		self->resources[typeId].resources.Clear();

	self->types[typeId].isCached = caching;
}

u32 ResourceSystem::GetResourceLoaderCount() const
{
	return (u32)self->loaders.Size();
}

StrongRef<ResourceLoader> ResourceSystem::GetResourceLoader(u32 id) const
{
	return self->loaders.At(id).loader;
}

void ResourceSystem::AddResourceWriter(ResourceWriter* writer)
{
	LX_CHECK_NULL_ARG(writer);

	log::Debug("Registered resource writer: ~s.", writer->GetName());
	self->writers.PushBack(writer);
}

u32 ResourceSystem::GetResourceWriterCount() const
{
	return self->writers.Size();
}

StrongRef<ResourceWriter> ResourceSystem::GetResourceWriter(Name resourceType, const String& ext) const
{
	for(auto it = self->writers.Last(); it != self->writers.Begin(); --it) {
		bool canWrite = (*it)->CanWriteType(ext, resourceType);
		if(canWrite)
			return *it;
	}

	return nullptr;
}

StrongRef<ResourceWriter> ResourceSystem::GetResourceWriter(u32 id) const
{
	return self->writers.At(id);
}

void ResourceSystem::WriteResource(Resource* resource, io::File* file, const String& ext)  const
{
	auto writer = GetResourceWriter(resource->GetReferableType(), ext);
	if(!writer)
		throw FileFormatException("File format not supported", ext.Data());

	writer->WriteResource(file, resource);
}

void ResourceSystem::WriteResource(Resource* resource, const io::Path& path) const
{
	auto file = io::FileSystem::Instance()->OpenFile(path, io::EFileMode::Write, true);
	auto ext = io::GetFileExtension(path);
	WriteResource(resource, file, ext);
}

Name ResourceSystem::GetFileType(io::File* file) const
{
	Name type;
	if(GetResourceLoader(type, file))
		return type;
	else
		return Name::INVALID;
}

void ResourceSystem::AddResourceLoader(ResourceLoader* loader)
{
	if(!loader)
		throw InvalidArgumentException("loader", "Must not be null");

	log::Debug("Registered resource loader: ~s.", loader->GetName());
	self->loaders.PushBack(loader);
}

u32 ResourceSystem::GetTypeCount() const
{
	return (u32)self->types.Size();
}

Name ResourceSystem::GetType(u32 id) const
{
	return self->types.At(id).name;
}

void ResourceSystem::AddType(Name name)
{
	TypeEntry entry(name);
	auto it = LinearSearch(entry, self->types);
	if(it != self->types.End())
		throw Exception("Resource type already exists");

	self->types.PushBack(entry);
	self->resources.PushBack(ResourceBlock());
	log::Debug("New resource type \"~s\".", name);
}

u32 ResourceSystem::GetTypeID(Name type) const
{
	TypeEntry entry(type);
	auto it = LinearSearch(entry, self->types);
	if(it == self->types.End())
		throw Exception("Resourcetype does not exist");

	return (u32)IteratorDistance(self->types.First(), it);
}

StrongRef<ResourceLoader> ResourceSystem::GetResourceLoader(Name& type, io::File* file) const
{
	const u32 fileCursor = file->GetCursor();
	StrongRef<ResourceLoader> result;
	for(auto it = self->loaders.Last(); it != self->loaders.Begin(); --it) {
		Name fileType = it->loader->GetResourceType(file, type);
		if(fileType != Name::INVALID) {
			if(type == Name::INVALID || type == fileType) {
				type = fileType;
				result = it->loader;
			}
		}

		// Reset file
		file->Seek(fileCursor, io::ESeekOrigin::Start);

		if(result != nullptr)
			break;
	}

	return result;
}

void ResourceSystem::LoadResource(const ResourceOrigin& origin, Resource* dst) const
{
	StrongRef<io::File> file = io::FileSystem::Instance()->OpenFile(origin.str);
	Name type = dst->GetReferableType();

	// Get loader and correct resource type from file
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file);
	if(!loader)
		throw FileFormatException("File format not supported", type.c_str());

	// Load the resource
	const u32 oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, dst);
		dst->SetLoaded(true);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
	}
}

StrongRef<Resource> ResourceSystem::CreateResource(Name type, io::File* file, const ResourceOrigin* origin)
{
	// Get loader and correct resource type from file
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file);
	if(!loader)
		throw FileFormatException("File format not supported", type.c_str());

	// Create the resource
	StrongRef<Resource> resource = self->refFactory->Create(type, origin).As<Resource>();
	if(!resource)
		throw InvalidArgumentException("type", "Is no valid resource type");

	// Load the resource
	const u32 oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, resource);
		resource->SetLoaded(true);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
	}

	return resource;
}

}
}