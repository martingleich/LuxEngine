#include "ResourceSystemImpl.h"
#include "io/FileSystem.h"
#include "io/File.h"

#include "resources/ResourceLoader.h"
#include "resources/ResourceWriter.h"

#include "core/ReferableFactory.h"

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
const Name Material = "material";
}

static u32 INVALID_ID = 0xFFFFFFFF;

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
		return resources.At(id).resource;
	}

	const string& GetName(u32 id) const
	{
		return resources.At(id).name;
	}

	u32 GetResourceId(Resource* resource) const
	{
		for(auto it = resources.First(); it != resources.End(); ++it) {
			if(it->resource == resource)
				return (u32)core::IteratorDistance(resources.First(), it);
		}

		return INVALID_ID;
	}

	u32 GetResourceId(const string& name) const
	{
		if(name.IsEmpty())
			return INVALID_ID;

		Entry entry(name, nullptr);
		auto it = core::BinarySearch(entry, resources.First(), resources.End());
		if(it == resources.End())
			return INVALID_ID;

		return (u32)core::IteratorDistance(resources.First(), it);
	}

	void AddResource(const string& name, Resource* resource)
	{
		lxAssert(resource);
		if(!resource)
			throw core::InvalidArgumentException("resource", "Must not be null");

		if(name.IsEmpty())
			throw core::InvalidArgumentException("name", "Must not be empty");

		Entry entry(name, resource);
		core::array<Entry>::Iterator n;
		auto it = core::BinarySearch(entry, resources.First(), resources.End(), &n);
		if(it != resources.End())
			throw core::Exception("Resource already exists");

		resources.Insert(entry, n);
	}

	void RemoveResource(u32 id)
	{
		if(id >= resources.Size())
			throw core::OutOfRangeException();

		resources.Erase(core::AdvanceIterator(resources.First(), id), true);
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
	core::array<StrongRef<ResourceWriter>> writers;

	core::array<TypeEntry> types;
	core::array<ResourceBlock> resources;

	io::FileSystem* fileSystem;
	core::ReferableFactory* refFactory;
};

ResourceSystemImpl::ResourceSystemImpl() :
	self(LUX_NEW(SelfType))
{
	self->fileSystem = io::FileSystem::Instance();
	self->refFactory = core::ReferableFactory::Instance();
}

ResourceSystemImpl::~ResourceSystemImpl()
{
	LUX_FREE(self);
}

u32 ResourceSystemImpl::GetResourceCount(Name type) const
{
	const u32 typeID = GetTypeID(type);
	if(typeID == INVALID_ID)
		throw core::InvalidArgumentException("type", "type does not exist");

	return (u32)self->resources[typeID].Size();
}

const string& ResourceSystemImpl::GetResourceName(Name type, u32 id) const
{
	const u32 typeId = GetTypeID(type);
	if(typeId > GetTypeCount())
		throw core::Exception("Type does not exist");

	return self->resources[typeId].GetName(id);
}

u32 ResourceSystemImpl::GetResourceId(Resource* resource) const
{
	if(!resource)
		throw core::InvalidArgumentException("resource", "Must not be null");

	const u32 typeId = GetTypeID(resource->GetReferableSubType());

	const u32 resId = self->resources[typeId].GetResourceId(resource);
	if(resId == INVALID_ID)
		throw core::Exception("Resource does not exist");

	return resId;
}

u32 ResourceSystemImpl::GetResourceId(Name type, const string& name) const
{
	u32 id = GetResourceIdUnsafe(type, name);
	if(id == INVALID_ID)
		throw core::ObjectNotFoundException(name.Data());

	return id;
}

u32 ResourceSystemImpl::GetResourceIdUnsafe(Name type, const string& name) const
{
	if(name.IsEmpty())
		throw core::InvalidArgumentException("name", "Must not be empty");

	const u32 typeId = GetTypeID(type);

	if(self->types[typeId].isCached == false)
		return INVALID_ID; 

	if(self->fileSystem->ExistFile(name)) {
		const string abs_path = self->fileSystem->GetAbsoluteFilename(name);
		return self->resources[typeId].GetResourceId(abs_path);
	} else {
		return self->resources[typeId].GetResourceId(name);
	}
}

void ResourceSystemImpl::AddResource(const string& name, Resource* resource)
{
	if(!resource)
		throw core::InvalidArgumentException("resource", "Must not be null");

	const u32 typeId = GetTypeID(resource->GetReferableSubType());
	if(self->types[typeId].isCached == false)
		return;

	self->resources[typeId].AddResource(name, resource);
}

void ResourceSystemImpl::RemoveResource(Name type, u32 id)
{
	const u32 typeId = GetTypeID(type);

	self->resources[typeId].RemoveResource(id);
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

	return self->resources[typeId].GetResource(id);
}

StrongRef<Resource> ResourceSystemImpl::GetResource(Name type, const string& name)
{
	if(name.IsEmpty())
		return nullptr;

	const u32 id = GetResourceIdUnsafe(type, name);
	if(id != INVALID_ID)
		return GetResource(type, id);

	StrongRef<io::File> file = self->fileSystem->OpenFile(name);

	auto out = GetResource(type, file);

	ResourceOrigin origin;
	origin.str = name;
	out->SetOrigin(nullptr, origin);

	return out;
}

StrongRef<Resource> ResourceSystemImpl::GetResource(Name type, io::File* file)
{
	if(!file)
		throw core::InvalidArgumentException("file", "Must not be null");

	StrongRef<Resource> resource = CreateResource(type, file);

	AddResource(file->GetName(), resource);

	ResourceOrigin origin;
	origin.str = file->GetName();
	resource->SetOrigin(nullptr, origin);
	resource->SetLoaded(true);

	return resource;
}

StrongRef<Resource> ResourceSystemImpl::CreateResource(Name type, const string& name)
{
	if(name.IsEmpty())
		return nullptr;

	const u32 id = GetResourceIdUnsafe(type, name);
	if(id != INVALID_ID)
		return GetResource(type, id);

	StrongRef<io::File> file = self->fileSystem->OpenFile(name);
	return CreateResource(type, file);
}

StrongRef<Resource> ResourceSystemImpl::CreateResource(Name type, io::File* file)
{
	// Get loader and correct resource type from file
	StrongRef<ResourceLoader> loader = GetResourceLoader(type, file);
	if(!loader)
		throw core::FileFormatException("File format not supported", type.c_str());

	// TODO: Use origin and originloader to load data
	StrongRef<Resource> resource = self->refFactory->Create(ReferableType::Resource, type).As<Resource>();

	const u32 oldCursor = file->GetCursor();
	try {
		loader->LoadResource(file, resource);
	} catch(...) {
		file->Seek(oldCursor, io::ESeekOrigin::Start);
		throw;
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
	return self->loaders.At(id).loader;
}

void ResourceSystemImpl::AddResourceWriter(ResourceWriter* writer)
{
	LX_CHECK_NULL_ARG(writer);

	log::Debug("Registered resource writer: ~s.", writer->GetName());
	self->writers.PushBack(writer);
}

u32 ResourceSystemImpl::GetResourceWriterCount() const
{
	return self->writers.Size();
}

StrongRef<ResourceWriter> ResourceSystemImpl::GetResourceWriter(core::Name resourceType, const string& ext) const
{
	for(auto it = self->writers.Last(); it != self->writers.Begin(); --it) {
		bool canWrite = (*it)->CanWriteType(ext, resourceType);
		if(canWrite)
			return *it;
	}

	return nullptr;
}

StrongRef<ResourceWriter> ResourceSystemImpl::GetResourceWriter(u32 id) const
{
	return self->writers.At(id);
}

void ResourceSystemImpl::WriteResource(Resource* resource,  io::File* file, const string& ext)  const
{
	auto writer = GetResourceWriter(resource->GetReferableSubType(), ext);
	if(!writer)
		throw core::FileFormatException("File format not supported", ext.Data());

	writer->WriteResource(file, resource);
}

void ResourceSystemImpl::WriteResource(Resource* resource, const io::path& path) const
{
	auto file = io::FileSystem::Instance()->OpenFile(path, io::EFileMode::Write, true);
	auto ext = io::GetFileExtension(path);
	WriteResource(resource, file, ext);
}

core::Name ResourceSystemImpl::GetFileType(io::File* file) const
{
	core::Name type;
	if(GetResourceLoader(type, file))
		return type;
	else
		return core::Name::INVALID;
}

void ResourceSystemImpl::AddResourceLoader(ResourceLoader* loader)
{
	if(!loader)
		throw core::InvalidArgumentException("loader", "Must not be null");

	log::Debug("Registered resource loader: ~s.", loader->GetName());
	self->loaders.PushBack(loader);
}

u32 ResourceSystemImpl::GetTypeCount() const
{
	return (u32)self->types.Size();
}

Name ResourceSystemImpl::GetType(u32 id) const
{
	return self->types.At(id).name;
}

void ResourceSystemImpl::AddType(Name name)
{
	TypeEntry entry(name);
	auto it = core::LinearSearch(entry, self->types.First(), self->types.End());
	if(it != self->types.End())
		throw core::Exception("Resource type already exists");

	self->types.PushBack(entry);
	self->resources.PushBack(ResourceBlock());
	log::Debug("New resource type \"~s\".", name);
}

u32 ResourceSystemImpl::GetTypeID(Name type) const
{
	TypeEntry entry(type);
	auto it = core::LinearSearch(entry, self->types.First(), self->types.End());
	if(it == self->types.End())
		throw core::Exception("Resourcetype does not exist");

	return (u32)core::IteratorDistance(self->types.First(), it);
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

}
}