#include "core/ParamPackage.h"

namespace lux
{
namespace core
{

const ParamPackage ParamPackage::EMPTY;

ParamPackage::ParamPackage() :
	m_IsTrivial(true)
{
}
ParamPackage::ParamPackage(const CreateEntry* createEntries, int paramCount)
{
	if(paramCount == 0)
		return;
	m_Params.Resize(paramCount);

	int stringSize = 0;
	int defaultSize = 0;
	bool isTrivial = true;
	for(int i = 0; i < paramCount; ++i) {
		lxAssert(!createEntries[i].type.IsUnknown());

		isTrivial &= createEntries[i].type.IsTrivial();
		int align = createEntries[i].type.GetAlign();
		lxAssert(align <= alignof(std::max_align_t));

		int size = createEntries[i].type.GetSize();
		defaultSize = (defaultSize + align - 1) / align * align;
		defaultSize += size;

		stringSize += createEntries[i].name.Size();
	}
	m_IsTrivial = isTrivial;
	m_Strings.SetSize(stringSize);
	m_DefaultData.SetSize(defaultSize);

	// Copy the data.
	int valueCursor = 0;
	int strCursor = 0;
	for(int i = 0; i < paramCount; ++i) {
		ParamEntry& outParam = m_Params[i];
		outParam.strEntry = strCursor;
		outParam.strLength = createEntries[i].name.Size();
		std::memcpy(
			(char*)m_Strings + strCursor,
			createEntries[i].name.Data(),
			createEntries[i].name.Size());
		strCursor += outParam.strLength;

		int align = createEntries[i].type.GetAlign();
		int size = createEntries[i].type.GetSize();
		valueCursor = (valueCursor + align - 1) / align * align;
		outParam.valueOffset = valueCursor;
		valueCursor += size;
		outParam.type = createEntries[i].type;
		lxAssert((int)createEntries[i].defaultValue.GetSize() >= size);
		outParam.type.CopyConstruct(
			(u8*)m_DefaultData + outParam.valueOffset,
			createEntries[i].defaultValue);
	}
}
ParamPackage::~ParamPackage()
{
	if(!m_IsTrivial) {
		// Release default values
		for(int i = 0; i < GetParamCount(); ++i) {
			auto& param = m_Params[i];
			param.type.Destruct((u8*)m_DefaultData + param.valueOffset);
		}
	}
}
ParamPackage::ParamPackage(const ParamPackage& other)
{
	*this = other;
}
ParamPackage::ParamPackage(ParamPackage&& old)
{
	*this = std::move(old);
}
ParamPackage& ParamPackage::operator=(const ParamPackage& other)
{
	lxAssertEx(m_Params.IsEmpty(), "Can only assign to empty param package.");
	if(!m_IsTrivial) {
		// Release default values
		for(int i = 0; i < GetParamCount(); ++i) {
			auto& param = m_Params[i];
			param.type.Destruct((u8*)m_DefaultData + param.valueOffset);
		}
	}

	m_Params = other.m_Params;
	m_IsTrivial = other.m_IsTrivial;

	m_Strings = other.m_Strings;
	m_DefaultData.SetSize(other.m_DefaultData.GetSize());

	// Copy paramdata and default values.
	for(int i = 0; i < GetParamCount(); ++i) {
		ParamEntry& outParam = m_Params[i];
		outParam = other.m_Params[i];

		outParam.type.CopyConstruct(
			(u8*)m_DefaultData + outParam.valueOffset,
			(const u8*)other.m_DefaultData + outParam.valueOffset);
	}

	return *this;
}

ParamPackage& ParamPackage::operator=(ParamPackage&& old)
{
	lxAssertEx(m_Params.IsEmpty(), "Can only assign to empty param package.");
	if(!m_IsTrivial) {
		// Release default values
		for(int i = 0; i < GetParamCount(); ++i) {
			auto& param = m_Params[i];
			param.type.Destruct((u8*)m_DefaultData + param.valueOffset);
		}
	}

	m_Params = std::move(old.m_Params);
	m_IsTrivial = old.m_IsTrivial;

	m_Strings = std::move(old.m_Strings);

	// This will reuse the old memory, so no individual moves are necessary.
	auto oldMem = old.m_DefaultData.Pointer();
	m_DefaultData = std::move(old.m_DefaultData);
	lxAssert(oldMem == m_DefaultData.Pointer());
	(void)oldMem;
	return *this;
}

void* ParamPackage::CreatePackage() const
{
	if(GetPackSize() == 0)
		return nullptr;
	u8* out = LUX_NEW_ARRAY(u8, GetPackSize());
	if(m_IsTrivial) {
		std::memcpy(out, m_DefaultData, GetPackSize());
	} else {
		for(int i = 0; i < GetParamCount(); ++i) {
			auto& param = m_Params[i];
			param.type.CopyConstruct(
				out + param.valueOffset,
				(const u8*)m_DefaultData + param.valueOffset);
		}
	}
	return out;
}
void ParamPackage::DestroyPackage(void* data) const
{
	if(GetPackSize() == 0)
		return;
	if(!m_IsTrivial) {
		for(int i = 0; i < GetParamCount(); ++i) {
			auto& param = m_Params[i];
			param.type.Destruct(((u8*)data) + param.valueOffset);
		}
	}
	LUX_FREE_ARRAY((u8*)data);
}
bool ParamPackage::ComparePackage(const void* a, const void* b) const
{
	for(int i = 0; i < GetParamCount(); ++i) {
		auto& param = m_Params[i];
		if(param.type.Compare((const u8*)a + param.valueOffset, (const u8*)b + param.valueOffset))
			return false;
	}

	return true;
}
void* ParamPackage::CopyPackage(const void* b) const
{
	u8* out = LUX_NEW_ARRAY(u8, GetPackSize());
	for(int i = 0; i < GetParamCount(); ++i) {
		auto& param = m_Params[i];
		param.type.CopyConstruct(out + param.valueOffset, (u8*)b + param.valueOffset);
	}
	return out;
}
void ParamPackage::AssignPackage(void* a, const void* b) const
{
	for(int i = 0; i < GetParamCount(); ++i) {
		auto& param = m_Params[i];
		param.type.Assign((u8*)a + param.valueOffset, (u8*)b + param.valueOffset);
	}
}

int ParamPackage::GetParamIdByName(StringView name) const
{
	for(int i = 0; i < GetParamCount(); ++i) {
		auto& param = m_Params[i];
		auto str = GetStr(param.strEntry);
		if(param.strLength == name.Size() && std::memcmp(str, name.Data(), param.strLength) == 0)
			return i;
	}

	return -1;
}

} // namespace core
} // namespace lux
