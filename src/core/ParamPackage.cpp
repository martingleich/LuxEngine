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

void ParamPackage::AddParam(core::Type type, const char* name, const void* defaultValue, u16 reserved)
{
	Entry entry;
	entry.name = name;
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

void ParamPackage::AddParam(const ParamDesc& desc)
{
	Entry entry;
	entry.name = desc.name;
	entry.size = desc.size;
	entry.type = desc.type;
	entry.reserved = desc.reserved;
	if(entry.type == core::Type::Unknown)
		throw Exception("Type is not supported in param package");

	AddEntry(entry, desc.defaultValue);
}

void* ParamPackage::CreatePackage() const
{
	u8* out = LUX_NEW_ARRAY(u8, self->TotalSize);
	memcpy(out, self->DefaultPackage, self->TotalSize);
	return out;
}

const void* ParamPackage::GetDefault() const
{
	return self->DefaultPackage;
}

ParamPackage::ParamDesc ParamPackage::GetParamDesc(u32 param) const
{
	if(param >= self->Params.Size())
		throw OutOfRangeException();

	auto& p = self->Params.At(param);
	ParamDesc desc;
	desc.name = p.name.Data();
	desc.size = p.size;
	desc.type = p.type;
	desc.reserved = p.reserved;
	desc.defaultValue = (const u8*)self->DefaultPackage + p.offset;

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

	return PackageParam(p.size, type, (u8*)baseData + p.offset, p.name.Data());
}

PackageParam ParamPackage::GetParamFromName(const string& name, void* baseData, bool isConst) const
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

	return PackageParam();
}

void ParamPackage::SetDefaultValue(u32 param, const void* defaultValue, core::Type type)
{
	if(!defaultValue)
		return;

	const Entry& e = self->Params.At(param);
	if(type == core::Type::Unknown || type == e.type)
		memcpy((u8*)self->DefaultPackage + e.offset, defaultValue, e.size);
	else {
		if(!IsConvertible(type, e.type))
			throw TypeException("Incompatible types used", type, e.type);
		ConvertBaseType(type, defaultValue, e.type, (u8*)self->DefaultPackage + e.offset);
	}
}

void ParamPackage::SetDefaultValue(const string& param, const void* defaultValue, core::Type type)
{
	SetDefaultValue(GetParamId(param), defaultValue, type);
}

u32 ParamPackage::GetParamId(const string& name, core::Type type) const
{
	for(u32 i = 0; i < self->Params.Size(); ++i) {
		if(self->Params[i].name == name) {
			if(type == core::Type::Unknown || self->Params[i].type == type)
				return i;
		}
	}

	throw ObjectNotFoundException(name.Data());
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

	self->DefaultPackage.SetMinSize(entry.offset + entry.size, core::mem::RawMemory::COPY);
	memcpy((u8*)self->DefaultPackage + entry.offset, defaultValue, entry.size);

	self->Params.PushBack(entry);
}


} // namespace core
} // namespace lux
