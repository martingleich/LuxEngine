#include "RendererNull.h"
#include "core/Logger.h"
#include "video/MaterialImpl.h"
#include "video/RenderTarget.h"

namespace lux
{
namespace video
{

RendererNull::RendererNull(VideoDriver* driver) :
	m_RenderMode(ERenderMode::None),
	m_DirtyFlags(0xFFFFFFFF), // Set all dirty flags at start
	m_Driver(driver),
	m_IsFogActive(false)
{
	m_NormalizeNormals = true;

	m_Params.AddAttribute("camPos", math::Vector3F(0, 0, 0));
	m_ParamId.lighting = m_Params.AddAttribute("lighting", (float)video::ELighting::Enabled);
	m_ParamId.ambient = m_Params.AddAttribute("ambient", video::Colorf(0, 0, 0));
	m_ParamId.time = m_Params.AddAttribute("time", 0.0f);

	m_ParamId.fog1 = m_Params.AddAttribute("fog1", video::Colorf(1, 1, 1, 0));
	m_ParamId.fog2 = m_Params.AddAttribute("fog2", video::Colorf(0, 0, 0, 0));

	for(size_t i = 0; i < m_MatrixTable.GetCount(); ++i)
		m_Params.AddAttribute(m_MatrixTable.CreateAttribute(i));

	for(size_t i = 0; i < 16; ++i)
		m_ParamId.lights.PushBack(m_Params.AddAttribute("light" + core::StringConverter::ToString(i), math::Matrix4::ZERO));

	m_RenderStatistics = RenderStatistics::Instance();
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::SetPass(const Pass& pass, bool useOverwrite, ParamSetCallback* paramSetCallback)
{
	SetDirty(Dirty_Material);

	m_Material = nullptr;
	m_PassId = 0;
	m_Pass = pass;
	m_ParamSetCallback = paramSetCallback;
	m_UseOverwrite = useOverwrite;
}

void RendererNull::SetMaterial(const Material* material, size_t passId)
{
	SetDirty(Dirty_Material);
	m_Material = material;
	m_PassId = passId;

	m_Pass = m_Material->GeneratePass(m_PassId);
	m_ParamSetCallback = m_Material->GetRenderer();

	m_UseOverwrite = true;
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
	SetDirty(Dirty_Overwrites);
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

	m_Lights.PushBack(light);
	SetDirty(Dirty_Lights);
}

void RendererNull::ClearLights()
{
	m_Lights.Clear();
	SetDirty(Dirty_Lights);
}

void RendererNull::SetFog(const FogData& fog)
{
	m_IsFogActive = true;
	m_Fog = fog;
	SetDirty(Dirty_Fog);
}

void RendererNull::ClearFog()
{
	m_IsFogActive = false;
	SetDirty(Dirty_Fog);
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::SetTransform(ETransform transform, const math::Matrix4& matrix)
{
	switch(transform) {
	case ETransform::World:
		m_TransformWorld = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_WORLD, matrix);
		SetDirty(Dirty_World);
		break;
	case ETransform::View:
		m_TransformView = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_VIEW, matrix);
		SetDirty(Dirty_ViewProj);
		break;
	case ETransform::Projection:
		m_TransformProj = matrix;
		m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, matrix);
		SetDirty(Dirty_ViewProj);
		break;
	}
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

void RendererNull::SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token)
{
	if(!token->renderer) {
		token->renderer = this;
		token->prev = m_NormalizeNormals;
	}
	m_NormalizeNormals = normalize;
	SetDirty(Dirty_Overwrites);
}

bool RendererNull::GetNormalizeNormals() const
{
	return m_NormalizeNormals;
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::AddParam(const String& name, core::Type type, const void* value)
{
	m_Params.AddAttribute(name, type, value);
}

core::AttributePtr RendererNull::GetParam(const String& name) const
{
	return m_Params.Pointer(name);
}

const core::Attributes& RendererNull::GetParams() const
{
	return m_Params;
}

///////////////////////////////////////////////////////////////////////////

VideoDriver* RendererNull::GetDriver() const
{
	return m_Driver;
}

///////////////////////////////////////////////////////////////////////////
/*
Lux illumination matrix.
Only the location of the t value is fixed, all other values depend on the light
type, for built-in point/directonal and spot light the matrix is build as
following:

 r  g  b t
px py pz 0
dx dy dz 0
ra ic oc 0

t = Type of light (0 = Disabled, 1 = Directional, 2 = Point, 3 = Spot)

(r,g,b) = Diffuse color of light
(px,py,pz) = Position of light

fa = Falloff for spotlight
ic = Cosine of half inner cone for spotlight
oc = Cosine of half outer cone for spotlight
*/
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

	matrix(3, 0) = data.falloff;
	matrix(3, 1) = cos(data.innerCone);
	matrix(3, 2) = cos(data.outerCone);
	matrix(3, 3) = 0.0f;

	matrix(1, 3) = 0.0f;
	matrix(2, 3) = 0.0f;

	return matrix;
}

} // namespace video
} // namespace lux
