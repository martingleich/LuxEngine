#include "core/ParamPackage.h"
#include "video/CubeTexture.h"
#include "video/Texture.h"

namespace lux
{
namespace core
{

struct ParamPackage::SelfData
{
	core::array<Entry> Params;

	u32 TotalSize;
	u32 TextureCount;

	core::mem::RawMemory DefaultPackage;
};

ParamPackage::ParamPackage() :
	self(new SelfData)
{
	self->TextureCount = 0;
	self->TotalSize = 0;
}

ParamPackage::~ParamPackage()
{
	delete self;
}

ParamPackage::ParamPackage(const ParamPackage& other) :
	self(new SelfData)
{
	self->TextureCount = other.self->TextureCount;
	self->TotalSize = other.self->TotalSize;
	self->Params = other.self->Params;
	self->DefaultPackage = other.self->DefaultPackage;
}

ParamPackage& ParamPackage::operator=(const ParamPackage& other)
{
	self->TextureCount = other.self->TextureCount;
	self->TotalSize = other.self->TotalSize;
	self->Params = other.self->Params;
	self->DefaultPackage = other.self->DefaultPackage;

	return *this;
}

void ParamPackage::Clear()
{
	self->TotalSize = 0;
	self->TextureCount = 0;
	self->Params.Clear();
}

void ParamPackage::AddParam(core::Type type, const string_type& name, const void* defaultValue, u16 reserved)
{
	u32 id;
	if(GetId(name, core::Type::Unknown, id))
		throw InvalidArgumentException("name", "Name is already used");

	Entry entry;
	entry.name = name.data;
	const u32 size = type.GetSize();

	// There are currently no types triggering this, just to be shure
	if(size > 255)
		throw Exception("Type is too big for param package");

	entry.size = (u8)size;
	entry.type = type;
	entry.reserved = reserved;
	if(entry.type == core::Type::Unknown)
		throw Exception("Type is not supported in param package");

	AddEntry(entry, defaultValue);
}

void ParamPackage::MergePackage(const ParamPackage& other)
{
	for(u32 i = 0; i < other.GetParamCount(); ++i) {
		auto desc = other.GetParamDesc(i);

		u32 id;
		if(GetId(desc.name, core::Type::Unknown, id)) {
			if(self->Params[id].type != desc.type)
				throw Exception("Same name with diffrent types in package merge");
			u16 reserved = 0xFFFF;
			if(self->Params[id].reserved != 0xFFFF)
				reserved = self->Params[id].reserved;
			if(desc.reserved != 0xFFFFF) {
				if(reserved != 0xFFFF)
					throw Exception("Same name with diffrent reserved values in package merge");
				else
					reserved = (u16)desc.reserved;
			}
			self->Params[id].reserved = reserved;
			continue;
		}

		Entry entry;
		entry.name = desc.name;
		entry.reserved = (u16)desc.reserved;
		entry.type = desc.type;
		entry.size = (u8)entry.type.GetSize();
		AddEntry(entry, desc.defaultValue);
	}
}

void* ParamPackage::CreatePackage() const
{
	u8* out = LUX_NEW_ARRAY(u8, self->TotalSize);
	for(auto it = self->Params.First(); it != self->Params.End(); ++it)
		it->type.CopyConstruct(out + it->offset, (u8*)self->DefaultPackage + it->offset);

	return out;
}

void ParamPackage::DestroyPackage(void* p) const
{
	for(auto it = self->Params.First(); it != self->Params.End(); ++it)
		it->type.Destruct((u8*)p + it->offset);

	LUX_FREE_ARRAY((u8*)p);
}

bool ParamPackage::ComparePackage(const void* a, const void* b) const
{
	for(auto it = self->Params.First(); it != self->Params.End(); ++it) {
		if(!it->type.Compare((const u8*)a + it->offset, (const u8*)b + it->offset))
			return false;
	}

	return true;
}

void* ParamPackage::CopyPackage(const void* b) const
{
	u8* out = LUX_NEW_ARRAY(u8, self->TotalSize);
	for(auto it = self->Params.First(); it != self->Params.End(); ++it)
		it->type.CopyConstruct(out + it->offset, (u8*)b + it->offset);

	return out;
}

ParamDesc ParamPackage::GetParamDesc(u32 param) const
{
	if(param >= self->Params.Size())
		throw OutOfRangeException();

	auto& p = self->Params.At(param);
	ParamDesc desc;
	desc.name = p.name.Data();
	desc.size = p.size;
	desc.type = p.type;
	desc.id = param;
	desc.reserved = p.reserved;
	desc.defaultValue = (u8*)self->DefaultPackage + p.offset;

	return desc;
}

const string& ParamPackage::GetParamName(u32 param) const
{
	return self->Params.At(param).name;
}

PackageParam ParamPackage::GetParam(u32 param, void* baseData, bool isConst) const
{
	auto& p = self->Params.At(param);

	core::Type type = isConst ? p.type.GetConstantType() : p.type;

	ParamDesc desc;
	desc.name = p.name.Data();
	desc.size = p.size;
	desc.type = type;
	desc.id = param;
	return PackageParam(desc, (u8*)baseData + p.offset);
}

PackageParam ParamPackage::GetParamFromName(const string_type& name, void* baseData, bool isConst) const
{
	return GetParam(GetParamId(name), baseData, isConst);
}

PackageParam ParamPackage::GetParamFromType(core::Type type, u32 index, void* baseData, bool isConst) const
{
	u32 id = 0;
	for(u32 i = 0; i < self->Params.Size(); ++i) {
		if(self->Params[i].type == type) {
			if(id == index)
				return GetParam(i, baseData, isConst);
			id++;
		}
	}

	throw core::ObjectNotFoundException("param_by_type");
}

void ParamPackage::SetDefaultValue(u32 param, const void* defaultValue, core::Type type)
{
	if(!defaultValue)
		return;

	const Entry& e = self->Params.At(param);
	if(type == core::Type::Unknown || type == e.type) {
		e.type.CopyConstruct((u8*)self->DefaultPackage + e.offset, defaultValue);
	} else {
		if(!IsConvertible(type, e.type))
			throw TypeException("Incompatible types used", type, e.type);
		ConvertBaseType(type, defaultValue, e.type, (u8*)self->DefaultPackage + e.offset);
	}
}

void ParamPackage::SetDefaultValue(const string_type& param, const void* defaultValue, core::Type type)
{
	SetDefaultValue(GetParamId(param), defaultValue, type);
}

u32 ParamPackage::GetParamId(const string_type& name, core::Type type) const
{
	u32 out;
	if(!GetId(name, type, out))
		throw ObjectNotFoundException(name.data);

	return out;
}

u32 ParamPackage::GetParamCount() const
{
	return (u32)self->Params.Size();
}

u32 ParamPackage::GetTextureCount() const
{
	return self->TextureCount;
}

u32 ParamPackage::GetTotalSize() const
{
	return self->TotalSize;
}

void ParamPackage::AddEntry(Entry& entry, const void* defaultValue)
{
	if(self->Params.Size() > 0)
		entry.offset = self->Params.Last()->offset + self->Params.Last()->size;
	else
		entry.offset = 0;

	self->TotalSize += entry.size;
	if(entry.type == core::Type::Texture)
		self->TextureCount++;

	core::mem::RawMemory newBlock(entry.offset + entry.size);
	for(auto it = self->Params.First(); it != self->Params.End(); ++it) {
		it->type.CopyConstruct((u8*)newBlock + it->offset, (u8*)self->DefaultPackage + it->offset);
		it->type.Destruct((u8*)self->DefaultPackage + it->offset);
	}

	entry.type.CopyConstruct((u8*)newBlock + entry.offset, defaultValue);

	self->Params.PushBack(entry);
	self->DefaultPackage = std::move(newBlock);
}

bool ParamPackage::GetId(string_type name, core::Type t, u32& outId) const
{
	string_type cname = name;
	cname.EnsureSize();
	for(outId = 0; outId < self->Params.Size(); ++outId) {
		if(self->Params[outId].name == cname) {
			if(t == core::Type::Unknown || self->Params[outId].type == t)
				return true;
		}
	}

	return false;
}

} // namespace core
} // namespace lux
