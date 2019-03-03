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
const Name Texture("lux.resource.Texture");
const Name CubeTexture("lux.resource.CubeTexture");
const Name Mesh("lux.resource.Mesh");
const Name Font("lux.resource.Font");
const Name Sound("lux.resource.Sound");
const Name Animation("lux.resource.Animation");
}

static int INVALID_ID = -1;

struct TypeEntry
{
	Name name;
	bool isCached;

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

	LoaderEntry(ResourceLoader* l) : loader(l)
	{
	}
};

using ResourceMap = core::HashMap<core::ResourceOrigin, StrongRef<core::Referable>>;
struct ResourceSystem::SelfType
{
	Array<LoaderEntry> loaders;
	Array<StrongRef<ResourceWriter>> writers;

	Array<TypeEntry> types;
	Array<ResourceMap> resources;

	io::FileSystem* fileSystem;
	ReferableFactory* refFactory;
};

static StrongRef<ResourceSystem> g_ResourceSystem;

class InternalResourceLoader : public OriginResourceLoader
{
public:
	void LoadResource(const String& origin, core::Referable* dst) const override
	{
		// Ask resource system for load.
		StrongRef<io::File> file = io::FileSystem::Instance()->OpenFile(io::Path(origin));
		Name type = dst->GetReferableType();

		// Get loader and correct resource type from file
		StrongRef<ResourceLoader> loader = g_ResourceSystem->GetResourceLoader(type, file, type);
		if(!loader)
			throw FileFormatException("File format not supported", type.AsView());

		// Load the resource
		auto oldCursor = file->GetCursor();
		try {
			loader->LoadResource(file, dst);
		} catch(...) {
			file->Seek(oldCursor, io::ESeekOrigin::Start);
			throw;
		}
	}

	const core::String& GetName() const override
	{
		return core::String::EMPTY;
	}
};

static InternalResourceLoader g_InternalResourceLoader;

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
	self(std::make_unique<SelfType>())
{
	self->fileSystem = io::FileSystem::Instance();
	self->refFactory = ReferableFactory::Instance();
}

ResourceSystem::~ResourceSystem()
{
}

int ResourceSystem::FreeUnusedResources(Name type)
{
	int count = 0;
	if(type.IsEmpty()) {
		// TODO:
	} else {
		// TODO:
	}

	return count;
}

StrongRef<core::Referable> ResourceSystem::GetResource(Name type, const io::Path& path, bool loadIfNotFound)
{
	auto typeId = GetTypeID(type);
	if(self->types[typeId].isCached) {
		auto absPath = self->fileSystem->GetAbsoluteFilename(path);
		if(IsBasePath(absPath)) {
			ResourceOrigin origin(&g_InternalResourceLoader, path.GetString());
			auto obj = self->resources[typeId].Get(origin, nullptr);
			if(obj)
				return obj;
		}
	}

	if(loadIfNotFound) {
		auto file = self->fileSystem->OpenFile(path);
		return CreateResource(typeId, file);
	}

	return nullptr;
}

StrongRef<core::Referable> ResourceSystem::GetResource(Name type, io::File* file, bool loadIfNotFound)
{
	LX_CHECK_NULL_ARG(file);

	auto& path = file->GetPath();
	auto typeId = GetTypeID(type);
	if(self->types[typeId].isCached && IsBasePath(path)) {
		ResourceOrigin origin(&g_InternalResourceLoader, path.GetString());
		auto obj = self->resources[typeId].Get(origin, nullptr);
		if(obj)
			return obj;
	}

	if(loadIfNotFound)
		return CreateResource(typeId, file);

	return nullptr;
}

StrongRef<core::Referable> ResourceSystem::CreateResource(int typeId, const io::Path& name)
{
	if(name.IsEmpty())
		throw GenericInvalidArgumentException("name", "Name may not be empty");

	auto file = self->fileSystem->OpenFile(name);
	return CreateResource(typeId, file);
}

StrongRef<core::Referable> ResourceSystem::CreateResource(int typeId, io::File* file)
{
	auto& path = file->GetPath();
	ResourceOrigin origin;
	if(IsBasePath(path))
		origin = ResourceOrigin(&g_InternalResourceLoader, path.GetString());
	return CreateResource(typeId, file, origin);
}

StrongRef<core::Referable> ResourceSystem::CreateResource(int typeId, io::File* file, const ResourceOrigin& origin)
{
	// Get loader and correct resource type from file
	auto type = self->types[typeId].name;
	core::Name typeToLoad;
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file, typeToLoad);
	if(!loader)
		throw FileFormatException("File format not supported", type.AsView());

	// Create the resource
	StrongRef<core::Referable> object = self->refFactory->Create(type, &origin);
	if(!object)
		throw GenericInvalidArgumentException("type", "Is no valid resource type");

	// Load the resource
	auto oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, object);

		// Add to cache
		if(self->types[typeId].isCached)
			self->resources[typeId].SetAndReplace(origin, object);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
	}

	return object;
}

void ResourceSystem::SetCaching(Name type, bool caching)
{
	auto typeId = GetTypeID(type);
	if(typeId >= self->types.Size())
		return;

	if(self->types[typeId].isCached == true && caching == false)
		self->resources[typeId].Clear();

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
	for(int i = self->writers.Size() - 1; i >= 0; --i) {
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

void ResourceSystem::WriteResource(core::Referable* resource, io::File* file, StringView ext)  const
{
	auto writer = GetResourceWriter(resource->GetReferableType(), ext);
	if(!writer)
		throw FileFormatException("File format not supported", ext);

	writer->WriteResource(file, resource);
}

void ResourceSystem::WriteResource(core::Referable* resource, const io::Path& path) const
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
	auto idOpt = self->types.LinearSearch(entry);
	if(idOpt.HasValue())
		throw ObjectAlreadyExistsException(name.AsView());

	self->types.PushBack(entry);
	self->resources.EmplaceBack();
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