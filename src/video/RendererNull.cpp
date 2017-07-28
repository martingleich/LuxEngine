#include "RendererNull.h"
#include "core/Logger.h"
#include "video/MaterialImpl.h"
#include "video/RenderTarget.h"

namespace lux
{
namespace video
{

RendererNull::RendererNull(VideoDriver* driver, DeviceState& deviceState) :
	m_DeviceState(deviceState),
	m_RenderMode(ERenderMode::None),
	m_DirtyFlags(0xFFFFFFFF), // Set all dirty flags at start
	m_Driver(driver)
{
	m_Fog.isActive = false;

	m_ParamId.lighting = AddParam("lighting", core::Types::Float());
	GetParam(m_ParamId.lighting) = 1.0f;
	m_ParamId.ambient = AddParam("ambient", core::Types::Colorf());
	GetParam(m_ParamId.ambient) = video::Colorf(0.0f, 0.0f, 0.0f);
	m_ParamId.time = AddParam("time", core::Types::Float());
	GetParam(m_ParamId.time) = 0.0f;

	m_ParamId.fogColor = AddInternalParam("fogColor", core::Types::Colorf());

	m_ParamId.fogRange = AddInternalParam("fogRange", core::Types::Vector3f());
	m_ParamId.fogInfo = AddInternalParam("fogInfo", core::Types::Vector3f());

	m_RenderStatistics = LUX_NEW(RenderStatistics);

	m_Material = LUX_NEW(MaterialImpl)(nullptr);
	m_InvalidMaterial = LUX_NEW(MaterialImpl)(nullptr);
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::SetMaterial(const Material* material)
{
	SetDirty(Dirty_Material);
	if(m_Material->GetRenderer() != material->GetRenderer())
		SetDirty(Dirty_MaterialRenderer);

	if(material->GetRenderer())
		m_Material->CopyFrom(material);
	else
		m_Material->CopyFrom(m_InvalidMaterial);
}

const Material* RendererNull::GetMaterial() const
{
	return m_Material;
}

void RendererNull::SetInvalidMaterial(const Material* material)
{
	m_InvalidMaterial->CopyFrom(material);
}

const Material* RendererNull::GetInvalidMaterial()
{
	return m_InvalidMaterial;
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::PushPipelineOverwrite(const PipelineOverwrite& over, PipelineOverwriteToken* token)
{
	m_PipelineOverwrites.PushBack(over);
	if(token) {
		lxAssert(token->renderer == nullptr || token->renderer == this);
		token->renderer = this;
		token->count++;
	}
	UpdatePipelineOverwrite();
}

void RendererNull::PopPipelineOverwrite(PipelineOverwriteToken* token)
{
	if(token) {
		lxAssert(token->renderer == this);
		lxAssert(token->count > 0);
		token->count--;
	}
	m_PipelineOverwrites.PopBack();
	UpdatePipelineOverwrite();
}

void RendererNull::UpdatePipelineOverwrite()
{
	if(m_PipelineOverwrites.IsEmpty()) {
		m_FinalOverwrite = PipelineOverwrite();
		return;
	}

	m_FinalOverwrite = m_PipelineOverwrites[0];
	for(size_t i = 1; i < m_PipelineOverwrites.Size(); ++i)
		m_FinalOverwrite.Append(m_PipelineOverwrites[i]);
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::AddLight(const LightData& light)
{
	if(m_Lights.Size() == GetMaxLightCount())
		throw core::ErrorException("Too many lights");

	// Fill remaining light objects
	for(size_t i = m_ParamId.lights.Size(); i < m_Lights.Size() + 1; ++i) {
		m_ParamId.lights.PushBack(
			AddInternalParam("light" + core::StringConverter::ToString(i), core::Types::Matrix()));
	}

	m_Lights.PushBack(light);
	SetDirty(Dirty_Lights);
}

void RendererNull::ClearLights()
{
	m_Lights.Clear();
	SetDirty(Dirty_Lights);
}

///////////////////////////////////////////////////////////////////////////
void RendererNull::SetFog(const FogData& fog)
{
	m_Fog = fog;
	SetDirty(Dirty_Fog);
}

const FogData& RendererNull::GetFog() const
{
	return m_Fog;
}

///////////////////////////////////////////////////////////////////////////
void RendererNull::SetTransform(ETransform transform, const math::Matrix4& matrix)
{
	switch(transform) {
	case ETransform::World:
		m_TransformWorld = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_WORLD, matrix);
		break;
	case ETransform::View:
		m_TransformView = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_VIEW, matrix);
		break;
	case ETransform::Projection:
		m_TransformProj = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, matrix);
		break;
	}

	SetDirty(Dirty_Transform);
}

const math::Matrix4& RendererNull::GetTransform(ETransform transform) const
{
	switch(transform) {
	case ETransform::World: return m_TransformWorld;
	case ETransform::View: return m_TransformView;
	case ETransform::Projection: return m_TransformProj;
	default: throw core::InvalidArgumentException("transform");
	}
}

///////////////////////////////////////////////////////////////////////////

u32 RendererNull::AddParam(const StringType& name, core::Type type)
{
	return AddParamEx(name, type, false);
}

u32 RendererNull::AddInternalParam(const StringType& name, core::Type type)
{
	return AddParamEx(name, type, true);
}

u32 RendererNull::AddParamEx(const StringType& name, core::Type type, bool isInternal)
{
	// Check for name in matrices
	u32 id;
	if(m_MatrixTable.GetParamIdByName(name.data, id))
		throw core::InvalidArgumentException("name", "Name already used");

	m_InternalTable.PushBack(isInternal);
	u32 offset = m_MatrixTable.GetCount();
	return m_ParamList.AddParam(name, type) + offset;
}

core::PackageParam RendererNull::GetParamEx(u32 id, bool internal)
{
	if(id < m_MatrixTable.GetCount()) {
		return m_MatrixTable.GetParamById(id);
	} else if(id < m_MatrixTable.GetCount() + m_ParamList.Size()) {
		u32 real_id = id - m_MatrixTable.GetCount();
		core::PackageParam p = m_ParamList[real_id];
		core::ParamDesc desc = p.GetDesc();
		if(m_InternalTable[desc.id] && !internal)
			desc.type = desc.type.GetConstantType();
		desc.id = id;
		return core::PackageParam(desc, (u8*)p.Pointer());
	} else {
		throw core::OutOfRangeException();
	}
}

core::PackageParam RendererNull::GetParam(u32 id)
{
	return GetParamEx(id, false);
}

core::PackageParam RendererNull::GetParamInternal(u32 id)
{
	return GetParamEx(id, true);
}

core::PackageParam RendererNull::GetParam(const StringType& string)
{
	u32 id;
	if(m_MatrixTable.GetParamIdByName(string.data, id))
		return GetParam(id);
	else if(GetLightId(string, id))
		return GetParam(id);
	else
		return GetParam(m_ParamList.GetId(string) + m_MatrixTable.GetCount());
}

u32 RendererNull::GetParamCount() const
{
	return m_ParamList.Size() + m_MatrixTable.GetCount();
}

///////////////////////////////////////////////////////////////////////////

StrongRef<RenderStatistics> RendererNull::GetRenderStatistics() const
{
	return m_RenderStatistics;
}

VideoDriver* RendererNull::GetDriver() const
{
	return m_Driver;
}

///////////////////////////////////////////////////////////////////////////
math::Matrix4 RendererNull::GenerateLightMatrix(const LightData& data, bool active)
{
	math::Matrix4 matrix;

	if(!active) {
		matrix(0, 3) = 0.0f;
		return matrix;
	}

	if(data.type == ELightType::Directional)
		matrix(0, 3) = 1.0f;
	else if(data.type == ELightType::Point)
		matrix(0, 3) = 2.0f;
	else if(data.type == ELightType::Spot)
		matrix(0, 3) = 3.0f;
	else
		throw core::Exception("Unknown data type");

	matrix(0, 0) = data.color.r;
	matrix(0, 1) = data.color.g;
	matrix(0, 2) = data.color.b;

	matrix(1, 0) = data.position.x;
	matrix(1, 1) = data.position.y;
	matrix(1, 2) = data.position.z;

	matrix(2, 0) = data.direction.x;
	matrix(2, 1) = data.direction.y;
	matrix(2, 2) = data.direction.z;

	matrix(3, 0) = data.range;
	matrix(3, 1) = data.falloff;
	matrix(3, 2) = data.innerCone;
	matrix(3, 3) = data.outerCone;

	matrix(1, 3) = 0.0f;
	matrix(2, 3) = 0.0f;

	return matrix;
}

bool RendererNull::GetLightId(const StringType& string, u32& outId)
{
	if(strncmp(string.data, "light", 5) != 0)
		return false;

	const char* num = string.data + 5;
	const char* end;
	int id = core::StringConverter::ParseInt(num, -1, &end);
	if(id < 0)
		return false;
	if(*end != 0)
		return false;

	if((size_t)id >= GetMaxLightCount())
		return false;

	// Fill remaining light objects
	for(size_t i = m_ParamId.lights.Size(); i < (size_t)id + 1; ++i) {
		m_ParamId.lights.PushBack(
			AddInternalParam("light" + core::StringConverter::ToString(i), core::Types::Matrix()));
	}

	// Use the param id as id
	outId = m_ParamId.lights[id];

	return true;
}

} // namespace video
} // namespace lux
