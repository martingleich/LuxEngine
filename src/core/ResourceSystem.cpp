#include "core/ResourceSystem.h"
#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"
#include "core/Logger.h"

#include "io/FileSystem.h"
#include "io/File.h"
#include "io/Archive.h"

namespace lux
{
namespace core
{

namespace ResourceType
{
const Name Image("lux.resource.Image");
const Name ImageList("lux.resource.Imagelist");
const Name Texture("lux.resource.Texture");
const Name CubeTexture("lux.resource.CubeTexture");
const Name Mesh("lux.resource.Mesh");
const Name Font("lux.resource.Font");
const Name Sound("lux.resource.Sound");
const Name Animation("lux.resource.Animation");
}

static int INVALID_ID = -1;

struct Entry
{
	core::String path;
	StrongRef<Resource> resource;

	Entry()
	{
	}
	Entry(const core::String& n, Resource* r) : path(n), resource(r)
	{
	}

	bool operator<(const Entry& other) const
	{
		return path < other.path;
	}

	bool operator==(const Entry& other) const
	{
		return path == other.path;
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

	int Size() const
	{
		return resources.Size();
	}

	StrongRef<Resource> GetResource(int id)
	{
		return resources.At(id).resource;
	}

	int GetResourceId(const String& name) const
	{
		if(name.IsEmpty())
			return INVALID_ID;

		Entry entry(name, nullptr);
		auto it = BinarySearch(entry, resources);
		if(it == resources.End())
			return INVALID_ID;

		return IteratorDistance(resources.First(), it);
	}

	void AddResource(const String& name, Resource* resource)
	{
		lxAssert(resource);
		LX_CHECK_NULL_ARG(resource);

		if(name.IsEmpty())
			throw GenericInvalidArgumentException("name", "Must not be empty");

		Entry entry(name, resource);
		Array<Entry>::Iterator n;
		auto it = BinarySearch(entry, resources, &n);
		if(it != resources.End())
			throw ObjectAlreadyExistsException(name.AsView());

		resources.Insert(entry, core::IteratorDistance(resources.First(), n));
	}

	int RemoveUnused()
	{
		auto oldCount = resources.Size();
		resources.RemoveIf([](const Entry& e) -> bool { return e.resource->GetReferenceCount() == 1; });
		auto newCount = resources.Size();
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
	lxAssert(g_ResourceSystem);
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

int ResourceSystem::FreeUnusedResources(Name type)
{
	int count = 0;
	if(type.IsEmpty()) {
		for(auto it = self->resources.First(); it != self->resources.End(); ++it)
			count += it->RemoveUnused();
	} else {
		auto typeId = GetTypeID(type);
		if(typeId > GetTypeCount())
			return 0;
		count += self->resources[typeId].RemoveUnused();
	}

	return count;
}

StrongRef<Resource> ResourceSystem::GetResource(Name type, const io::Path& path, bool loadIfNotFound)
{
	auto typeId = GetTypeID(type);
	if(self->types[typeId].isCached) {
		auto absPath = self->fileSystem->GetAbsoluteFilename(path);
		if(IsBasePath(absPath)) {
			auto id = self->resources[typeId].GetResourceId(absPath.GetString().AsView());
			if(id != INVALID_ID)
				return self->resources[typeId].GetResource(id);
		}
	}

	StrongRef<Resource> resource;
	if(loadIfNotFound) {
		auto file = self->fileSystem->OpenFile(path);
		resource = CreateResource(typeId, file);

		// Add to cache
		if(self->types[typeId].isCached && resource->GetOrigin().loader == this)
			self->resources[typeId].AddResource(resource->GetOrigin().str, resource);
	}

	return resource;
}

StrongRef<Resource> ResourceSystem::GetResource(Name type, io::File* file, bool loadIfNotFound)
{
	LX_CHECK_NULL_ARG(file);

	auto& path = file->GetPath();
	auto typeId = GetTypeID(type);
	if(self->types[typeId].isCached && IsBasePath(path)) {
		auto id = self->resources[typeId].GetResourceId(path.AsView());
		if(id != INVALID_ID)
			return self->resources[typeId].GetResource(id);
	}

	if(loadIfNotFound) {
		auto resource = CreateResource(typeId, file);

		// Add to cache
		if(self->types[typeId].isCached && resource->GetOrigin().loader == this)
			self->resources[typeId].AddResource(resource->GetOrigin().str, resource);
		return resource;
	}

	return nullptr;
}

StrongRef<Resource> ResourceSystem::CreateResource(int typeId, const io::Path& name)
{
	if(name.IsEmpty())
		throw GenericInvalidArgumentException("name", "Name may not be empty");

	auto file = self->fileSystem->OpenFile(name);
	return CreateResource(typeId, file);
}

StrongRef<Resource> ResourceSystem::CreateResource(int typeId, io::File* file)
{
	auto& path = file->GetPath();
	ResourceOrigin origin;
	if(IsBasePath(path))
		origin = ResourceOrigin(this, path.GetString());
	return CreateResource(typeId, file, origin);
}

StrongRef<Resource> ResourceSystem::CreateResource(int typeId, io::File* file, const ResourceOrigin& origin)
{
	// Get loader and correct resource type from file
	auto type = self->types[typeId].name;
	core::Name typeToLoad;
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file, typeToLoad);
	if(!loader)
		throw FileFormatException("File format not supported", type.AsView());

	// Create the resource
	StrongRef<Resource> resource = self->refFactory->Create(type, &origin).As<Resource>();
	if(!resource)
		throw GenericInvalidArgumentException("type", "Is no valid resource type");

	// Load the resource
	auto oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, resource);
		resource->SetLoaded(true);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
	}

	return resource;
}

void ResourceSystem::SetCaching(Name type, bool caching)
{
	auto typeId = GetTypeID(type);
	if(typeId >= self->types.Size())
		return;

	if(self->types[typeId].isCached == true && caching == false)
		self->resources[typeId].resources.Clear();

	self->types[typeId].isCached = caching;
}

int ResourceSystem::GetResourceLoaderCount() const
{
	return self->loaders.Size();
}

StrongRef<ResourceLoader> ResourceSystem::GetResourceLoader(int id) const
{
	return self->loaders.At(id).loader;
}

void ResourceSystem::AddResourceWriter(ResourceWriter* writer)
{
	LX_CHECK_NULL_ARG(writer);

	log::Debug("Registered resource writer: {0}.", writer->GetName());
	self->writers.PushBack(writer);
}

int ResourceSystem::GetResourceWriterCount() const
{
	return self->writers.Size();
}

StrongRef<ResourceWriter> ResourceSystem::GetResourceWriter(Name resourceType, StringView ext) const
{
	for(int i = self->writers.Size()-1; i >= 0; --i) {
		bool canWrite = self->writers[i]->CanWriteType(ext, resourceType);
		if(canWrite)
			return self->writers[i];
	}

	return nullptr;
}

StrongRef<ResourceWriter> ResourceSystem::GetResourceWriter(int id) const
{
	return self->writers.At(id);
}

void ResourceSystem::WriteResource(Resource* resource, io::File* file, StringView ext)  const
{
	auto writer = GetResourceWriter(resource->GetReferableType(), ext);
	if(!writer)
		throw FileFormatException("File format not supported", ext);

	writer->WriteResource(file, resource);
}

void ResourceSystem::WriteResource(Resource* resource, const io::Path& path) const
{
	auto file = io::FileSystem::Instance()->OpenFile(path, io::EFileModeFlag::Write, true);
	auto ext = path.GetFileExtension();
	WriteResource(resource, file, ext);
}

Name ResourceSystem::GetFileType(io::File* file) const
{
	Name type;
	if(GetResourceLoader(type, file, type))
		return type;
	else
		return Name::INVALID;
}

void ResourceSystem::AddResourceLoader(ResourceLoader* loader)
{
	LX_CHECK_NULL_ARG(loader);

	log::Debug("Registered resource loader: {0}.", loader->GetName());
	self->loaders.PushBack(loader);
}

int ResourceSystem::GetTypeCount() const
{
	return self->types.Size();
}

Name ResourceSystem::GetType(int id) const
{
	return self->types.At(id).name;
}

void ResourceSystem::AddType(Name name)
{
	TypeEntry entry(name);
	auto it = LinearSearch(entry, self->types);
	if(it != self->types.End())
		throw ObjectAlreadyExistsException(name.AsView());

	self->types.PushBack(entry);
	self->resources.PushBack(ResourceBlock());
	log::Debug("New resource type \"{0}\".", name);
}

StrongRef<ResourceLoader> ResourceSystem::GetResourceLoader(Name type, io::File* file, Name& typeToLoad) const
{
	auto fileCursor = file->GetCursor();
	StrongRef<ResourceLoader> result;
	for(int i = self->loaders.Size() - 1; i >= 0; --i) {
		auto& entry = self->loaders[i];
		try {
			Name fileType = entry.loader->GetResourceType(file, type);
			if(!fileType.IsEmpty()) {
				if(!type.IsEmpty() || type == fileType) {
					typeToLoad = fileType;
					result = entry.loader;
				}
			}
		} catch(...) {
		}

		// Reset file
		file->Seek(fileCursor, io::ESeekOrigin::Start);

		if(result != nullptr)
			break;
	}

	return result;
}

void ResourceSystem::LoadResource(const String& origin, Resource* dst) const
{
	StrongRef<io::File> file = io::FileSystem::Instance()->OpenFile(io::Path(origin));
	Name type = dst->GetReferableType();

	// Get loader and correct resource type from file
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file, type);
	if(!loader)
		throw FileFormatException("File format not supported", type.AsView());

	// Load the resource
	auto oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, dst);
		dst->SetLoaded(true);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
	}
}

int ResourceSystem::GetTypeID(Name type) const
{
	TypeEntry entry(type);
	auto it = LinearSearch(entry, self->types);
	if(it == self->types.End())
		throw ObjectNotFoundException(type.AsView());

	return IteratorDistance(self->types.First(), it);
}

bool ResourceSystem::IsBasePath(const io::Path& path) const
{
	return path.GetArchive() == nullptr || path.GetArchive() == self->fileSystem->GetRootArchive();
}
}
}