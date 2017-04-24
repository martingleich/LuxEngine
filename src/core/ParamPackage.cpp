#include "core/ParamPackage.h"
#include "video/CubeTexture.h"
#include "video/Texture.h"

namespace lux
{
namespace core
{

const PackageParam PackageParam::INVALID = PackageParam(0, core::Type::Unknown, nullptr, nullptr);

PackageParam::PackageParam(u8 size, core::Type type, u8* data, const char* name) :
	m_Size(size),
	m_Type(type),
	m_Data(data),
	m_Name(name)
{
}

PackageParam::PackageParam() :
	m_Size(0),
	m_Type(core::Type::Unknown),
	m_Data(nullptr),
	m_Name(nullptr)
{
}

PackageParam::PackageParam(const PackageParam& other) :
	m_Size(other.m_Size),
	m_Type(other.m_Type),
	m_Data(other.m_Data),
	m_Name(other.m_Name)
{
}

PackageParam::operator video::MaterialLayer()
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid())
		return *(video::MaterialLayer*)m_Data;

	return video::MaterialLayer();
}

PackageParam::operator video::BaseTexture*()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return ((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam::operator video::Texture*()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return (video::Texture*)((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam::operator video::CubeTexture*()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return (video::CubeTexture*)((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam::operator StrongRef<video::BaseTexture>()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return ((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam::operator StrongRef<video::Texture>()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return (video::Texture*)((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam::operator StrongRef<video::CubeTexture>()
{
	lxAssert(m_Type == core::Type::Texture);

	if(IsValid())
		return (video::CubeTexture*)((video::MaterialLayer*)m_Data)->texture;
	return nullptr;
}

PackageParam& PackageParam::operator=(video::BaseTexture* texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(video::Texture* texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = (video::BaseTexture*)texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(video::CubeTexture* texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = (video::BaseTexture*)texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(StrongRef<video::BaseTexture> texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = (video::BaseTexture*)texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(StrongRef<video::Texture> texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = (StrongRef<video::BaseTexture>)texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(StrongRef<video::CubeTexture> texture)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(texture)
			texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		((video::MaterialLayer*)m_Data)->texture = (StrongRef<video::BaseTexture>)texture;
	}

	return *this;
}

PackageParam& PackageParam::operator=(video::MaterialLayer& Layer)
{
	lxAssert(m_Type == core::Type::Texture);
	if(IsValid()) {
		if(Layer.texture)
			Layer.texture->Grab();
		if(((video::MaterialLayer*)m_Data)->texture)
			((video::MaterialLayer*)m_Data)->texture->Drop();
		*(video::MaterialLayer*)m_Data = Layer;
	}

	return *this;
}

PackageParam& PackageParam::operator=(const PackageParam& varVal)
{
	lxAssert(varVal.m_Type == m_Type);
	if(IsValid()) {
		if(m_Type == core::Type::Texture) {
			if(((video::MaterialLayer*)varVal.m_Data)->texture)
				((video::MaterialLayer*)varVal.m_Data)->texture->Grab();
			if(((video::MaterialLayer*)m_Data)->texture)
				((video::MaterialLayer*)m_Data)->texture->Drop();
		}

		memcpy(m_Data, varVal.m_Data, m_Size);
	}

	return *this;
}

PackageParam& PackageParam::Set(const PackageParam& otherParam)
{
	m_Size = otherParam.m_Size;
	m_Type = otherParam.m_Type;
	m_Data = otherParam.m_Data;
	m_Name = otherParam.m_Name;

	return *this;
}

bool PackageParam::IsValid() const
{
	return (m_Data != nullptr);
}

const char* PackageParam::GetName() const
{
	if(IsValid())
		return m_Name;
	else
		return "";
}

u32 PackageParam::GetSize() const
{
	return m_Size;
}

core::Type PackageParam::GetType() const
{
	return m_Type.GetBaseType();
}

void* PackageParam::Pointer()
{
	return m_Data;
}

///////////////////////////////////////////////////////////////////////////////

void ParamPackage::Clear()
{
	m_TotalSize = 0;
	m_TextureCount = 0;
	m_Params.Clear();
}

void ParamPackage::AddEntry(Entry& entry, const void* defaultValue)
{
	if(m_Params.Size() > 0)
		entry.offset = m_Params.Last()->offset + m_Params.Last()->size;
	else
		entry.offset = 0;

	m_TotalSize += entry.size;
	if(entry.type == core::Type::Texture)
		m_TextureCount++;

	m_DefaultPackage.SetMinSize(entry.offset + entry.size, core::mem::RawMemory::COPY);
	memcpy((u8*)m_DefaultPackage + entry.offset, defaultValue, entry.size);

	m_Params.Push_Back(entry);
}

ParamPackage::ParamPackage() :
	m_TotalSize(0),
	m_TextureCount(0)
{
}

ParamPackage::~ParamPackage()
{
}

ParamPackage::ParamPackage(const ParamPackage& other) :
	m_TotalSize(0),
	m_TextureCount(0)
{
	*this = other;
}

ParamPackage::ParamPackage(ParamPackage&& old) :
	m_Params(std::move(old.m_Params)),
	m_TotalSize(old.m_TotalSize),
	m_DefaultPackage(std::move(old.m_DefaultPackage)),
	m_TextureCount(old.m_TextureCount)
{
}

ParamPackage& ParamPackage::operator=(const ParamPackage& other)
{
	m_Params = other.m_Params;
	m_TotalSize = other.m_TotalSize;
	m_TextureCount = other.m_TextureCount;

	m_DefaultPackage.SetMinSize(other.m_TotalSize);
	memcpy(m_DefaultPackage, other.m_DefaultPackage, other.m_TotalSize);

	return *this;
}

ParamPackage& ParamPackage::operator=(ParamPackage&& old)
{
	m_DefaultPackage = std::move(old.m_DefaultPackage);
	m_Params = std::move(old.m_Params);
	m_TotalSize = old.m_TotalSize;
	m_TextureCount = old.m_TextureCount;

	return *this;
}

void ParamPackage::AddParam(core::Type type, const char* name, const void* defaultValue, u16 reserved)
{
	Entry entry;
	entry.name = name;
	const u32 size = type.GetSize();
	lxAssert(size <= 255);
	entry.size = (u8)size;
	entry.type = type;
	entry.reserved = reserved;
	if(entry.type == core::Type::Unknown) {
		lxAssertNeverReach("Unsupported type");
		return;
	}

	AddEntry(entry, defaultValue);
}

void ParamPackage::AddParam(const ParamDesc& desc)
{
	Entry entry;
	entry.name = desc.name;
	entry.size = desc.size;
	entry.type = desc.type;
	entry.reserved = desc.reserved;
	if(entry.type == core::Type::Unknown) {
		lxAssertNeverReach("Unsupported type");
		return;
	}

	AddEntry(entry, desc.defaultValue);
}

void* ParamPackage::CreatePackage() const
{
	u8* out = LUX_NEW_ARRAY(u8, m_TotalSize);
	memcpy(out, m_DefaultPackage, m_TotalSize);
	return out;
}

const void* ParamPackage::GetDefault() const
{
	return m_DefaultPackage;
}

bool ParamPackage::GetParamDesc(u32 param, ParamDesc& desc) const
{
	if(param >= m_Params.Size())
		return false;

	desc.name = m_Params[param].name.Data();
	desc.size = m_Params[param].size;
	desc.type = m_Params[param].type;
	desc.reserved = m_Params[param].reserved;
	desc.defaultValue = (const u8*)m_DefaultPackage + m_Params[param].offset;
	return true;
}

const string& ParamPackage::GetParamName(u32 param) const
{
	if(param >= m_Params.Size())
		return string::EMPTY;

	return m_Params[param].name;
}

PackageParam ParamPackage::GetParam(u32 param, void* baseData, bool isConst) const
{
	if(param >= m_Params.Size())
		return PackageParam::INVALID;

	core::Type type = isConst ? m_Params[param].type.GetConstantType() : m_Params[param].type;

	return PackageParam(m_Params[param].size, type, (u8*)baseData + m_Params[param].offset, m_Params[param].name.Data());
}

PackageParam ParamPackage::GetParamFromName(const string& name, void* baseData, bool isConst) const
{
	for(u32 i = 0; i < m_Params.Size(); ++i) {
		if(m_Params[i].name == name)
			return GetParam(i, baseData, isConst);
	}

	return PackageParam::INVALID;
}

PackageParam ParamPackage::GetParamFromType(core::Type type, int index, void* baseData, bool isConst) const
{
	int CurrLayer = -1;
	for(u32 i = 0; i < m_Params.Size(); ++i) {
		if(m_Params[i].type == type) {
			CurrLayer++;
			if(CurrLayer == index)
				return GetParam(i, baseData, isConst);
		}
	}

	return PackageParam::INVALID;
}

void ParamPackage::SetDefaultValue(u32 param, const void* defaultValue)
{
	if(param >= m_Params.Size() || !defaultValue)
		return;

	const Entry& e = m_Params[param];
	memcpy((u8*)m_DefaultPackage + e.offset, defaultValue, e.size);
}

u32 ParamPackage::GetParamCount() const
{
	return (u32)m_Params.Size();
}

u32 ParamPackage::GetTextureCount() const
{
	return m_TextureCount;
}

u32 ParamPackage::GetTotalSize() const
{
	return m_TotalSize;
}

///////////////////////////////////////////////////////////////////////////////


PackagePuffer::PackagePuffer(const ParamPackage* pack) :
	m_Pack(nullptr),
	m_Data(nullptr),
	m_MaxSize(0)
{
	SetType(pack);
}

PackagePuffer::PackagePuffer(const PackagePuffer& other) :
	m_Pack(nullptr),
	m_Data(nullptr),
	m_MaxSize(0)
{
	*this = other;
}

PackagePuffer::PackagePuffer(PackagePuffer&& old) :
	m_Pack(old.m_Pack),
	m_Data(old.m_Data)
{
	old.m_Data = nullptr;
}

PackagePuffer::~PackagePuffer()
{
	LUX_FREE_ARRAY(m_Data);
}

bool PackagePuffer::operator==(const PackagePuffer& other) const
{
	if(m_Pack != other.m_Pack)
		return false;

	return (memcmp(m_Data, other.m_Data, m_Pack->GetTotalSize()) == 0);
}

bool PackagePuffer::operator!=(const PackagePuffer& other) const
{
	return !(*this == other);
}

PackagePuffer& PackagePuffer::operator=(const PackagePuffer& other)
{
	if(!other.m_Pack) {
		m_Pack = other.m_Pack;
		return *this;
	}

	if(m_Pack != other.m_Pack) {
		if(other.m_Pack->GetTotalSize() > m_MaxSize) {
			LUX_FREE_ARRAY(m_Data);
			m_Data = LUX_NEW_ARRAY(u8, other.m_Pack->GetTotalSize());
			m_MaxSize = other.m_Pack->GetTotalSize();
		}
	}

	memcpy(m_Data, other.m_Data, other.m_Pack->GetTotalSize());
	m_Pack = other.m_Pack;

	return *this;
}

PackagePuffer& PackagePuffer::operator=(PackagePuffer&& old)
{
	LUX_FREE_ARRAY(m_Data);
	m_Pack = old.m_Pack;
	m_Data = old.m_Data;
	old.m_Data = nullptr;
	return *this;
}

void PackagePuffer::SetType(const ParamPackage* pack)
{
	if(!pack) {
		m_Pack = pack;
		return;
	}

	if(m_Pack != pack) {
		if(pack->GetTotalSize() > m_MaxSize) {
			LUX_FREE_ARRAY(m_Data);
			m_Data = LUX_NEW_ARRAY(u8, pack->GetTotalSize());
			m_MaxSize = pack->GetTotalSize();
		}

		memcpy(m_Data, pack->GetDefault(), pack->GetTotalSize());
		m_Pack = pack;
	}
}

const ParamPackage* PackagePuffer::GetType() const
{
	return m_Pack;
}

void PackagePuffer::Reset()
{
	if(m_Pack)
		memcpy(m_Data, m_Pack->GetDefault(), m_Pack->GetTotalSize());
}

PackageParam PackagePuffer::FromName(const string& name, bool isConst) const
{
	if(m_Pack)
		return m_Pack->GetParamFromName(name, m_Data, isConst);
	else
		return PackageParam::INVALID;
}

PackageParam PackagePuffer::FromType(core::Type type, u32 index, bool isConst) const
{
	if(m_Pack)
		return m_Pack->GetParamFromType(type, index, m_Data, isConst);
	else
		return PackageParam::INVALID;
}

PackageParam PackagePuffer::FromID(u32 id, bool isConst) const
{
	if(m_Pack)
		return m_Pack->GetParam(id, m_Data, isConst);
	else
		return PackageParam::INVALID;
}

u32 PackagePuffer::GetParamCount() const
{
	if(m_Pack)
		return m_Pack->GetParamCount();
	else
		return 0;
}

u32 PackagePuffer::GetTextureCount() const
{
	if(m_Pack)
		return m_Pack->GetTextureCount();
	else
		return 0;
}

}
}