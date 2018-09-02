#include "core/ParamPackage.h"
#include "video/TextureLayer.h"

namespace lux
{
namespace core
{

struct ParamPackage::SelfData
{
	core::Array<Entry> Params;

	int TotalSize;
	int TextureCount;

	core::RawMemory DefaultPackage;
};

ParamPackage::ParamPackage() :
	self(LUX_NEW(SelfData))
{
	self->TextureCount = 0;
	self->TotalSize = 0;
}

ParamPackage::~ParamPackage()
{
	LUX_FREE(self);
}

ParamPackage::ParamPackage(const ParamPackage& other) :
	self(LUX_NEW(SelfData))
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

int ParamPackage::AddParam(core::Type type, const core::StringView& name, const void* defaultValue)
{
	int id;
	if(GetId(name, core::Type::Unknown, id)) {
		if(self->Params[id].type != type)
			throw InvalidArgumentException("", "Param already exists with diffrent parameter.");
		return id;
	}

	Entry entry;
	entry.name = name;
	const int size = type.GetSize();

	// There are currently no types triggering this, just to be shure
	if(size > 255)
		throw Exception("Type is too big for param package");

	entry.size = (u8)size;
	entry.type = type;
	if(entry.type == core::Type::Unknown)
		throw Exception("Type is not supported in param package");

	return AddEntry(entry, defaultValue);
}

void ParamPackage::MergePackage(const ParamPackage& other)
{
	for(int i = 0; i < other.GetParamCount(); ++i) {
		auto desc = other.GetParamDesc(i);

		int id;
		if(GetId(desc.name, core::Type::Unknown, id)) {
			if(self->Params[id].type != desc.type)
				throw Exception("Same name with diffrent types in package merge");
			continue;
		}

		auto defaultValue = other.DefaultValue(i);
		Entry entry;
		entry.name = desc.name;
		entry.type = desc.type;
		entry.size = (u8)entry.type.GetSize();
		AddEntry(entry, defaultValue.Pointer());
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

ParamDesc ParamPackage::GetParamDesc(int param) const
{
	auto& p = self->Params.At(param);
	ParamDesc desc;
	desc.name = p.name.Data();
	desc.type = p.type;
	desc.id = param;

	return desc;
}

const core::String& ParamPackage::GetParamName(int param) const
{
	return self->Params.At(param).name;
}

VariableAccess ParamPackage::GetParam(int param, void* baseData, bool isConst) const
{
	auto& p = self->Params.At(param);

	core::Type type = isConst ? p.type.GetConstantType() : p.type;

	return VariableAccess(type, (u8*)baseData + p.offset);
}

VariableAccess ParamPackage::GetParamFromName(const core::StringView& name, void* baseData, bool isConst) const
{
	return GetParam(GetParamId(name), baseData, isConst);
}

VariableAccess ParamPackage::GetParamFromType(core::Type type, int index, void* baseData, bool isConst) const
{
	int id = 0;
	for(int i = 0; i < self->Params.Size(); ++i) {
		if(self->Params[i].type == type) {
			if(id == index)
				return GetParam(i, baseData, isConst);
			id++;
		}
	}

	throw core::ObjectNotFoundException("param_by_type");
}

VariableAccess ParamPackage::DefaultValue(int param)
{
	auto& p = self->Params.At(param);
	return VariableAccess(p.type, (u8*)self->DefaultPackage + p.offset);
}

VariableAccess ParamPackage::DefaultValue(int param) const
{
	auto& p = self->Params.At(param);
	return VariableAccess(p.type.GetConstantType(), (u8*)self->DefaultPackage + p.offset);
}

VariableAccess ParamPackage::DefaultValue(const core::StringView& param)
{
	return DefaultValue(GetParamId(param));
}

int ParamPackage::GetParamId(const core::StringView& name, core::Type type) const
{
	int out;
	if(!GetId(name, type, out))
		throw ObjectNotFoundException(name.Data());

	return out;
}

int ParamPackage::GetParamCount() const
{
	return self->Params.Size();
}

int ParamPackage::GetTextureCount() const
{
	return self->TextureCount;
}

int ParamPackage::GetTotalSize() const
{
	return self->TotalSize;
}

int ParamPackage::AddEntry(Entry& entry, const void* defaultValue)
{
	if(self->Params.Size() > 0)
		entry.offset = self->Params.Last()->offset + self->Params.Last()->size;
	else
		entry.offset = 0;

	// Move offset to next aligned point
	if(entry.offset % entry.type.GetAlign() != 0)
		entry.offset += (u8)(entry.type.GetAlign() - (entry.offset % entry.type.GetAlign()));

	self->TotalSize += entry.size;
	if(entry.type == core::Types::Texture())
		self->TextureCount++;

	core::RawMemory newBlock(entry.offset + entry.size);
	for(auto it = self->Params.First(); it != self->Params.End(); ++it) {
		it->type.CopyConstruct((u8*)newBlock + it->offset, (u8*)self->DefaultPackage + it->offset);
		it->type.Destruct((u8*)self->DefaultPackage + it->offset);
	}

	if(defaultValue)
		entry.type.CopyConstruct((u8*)newBlock + entry.offset, defaultValue);
	else
		entry.type.Construct((u8*)newBlock + entry.offset);

	self->Params.PushBack(entry);
	self->DefaultPackage = std::move(newBlock);

	return self->Params.Size() - 1;
}

bool ParamPackage::GetId(core::StringView name, core::Type t, int& outId) const
{
	core::StringView cname = name;
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
