#include "RendererNull.h"
#include "core/Logger.h"
#include "video/RenderTarget.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

RenderRequest RenderRequest::Geometry3D(const Geometry* geo)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
	rq.is3D = true;
	rq.bufferData.ib = geo->GetIndices();
	rq.bufferData.vb = geo->GetVertices();
	rq.indexed = geo->GetIndices();
	rq.firstPrimitive = 0;
	rq.primitiveCount = geo->GetPrimitiveCount();
	rq.primitiveType = geo->GetPrimitiveType();
	rq.frontFace = geo->GetFrontFaceWinding();
	return rq;
}
RenderRequest RenderRequest::Geometry2D(const Geometry* geo)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
	rq.is3D = false;
	rq.bufferData.ib = geo->GetIndices();
	rq.bufferData.vb = geo->GetVertices();
	rq.indexed = geo->GetIndices();
	rq.firstPrimitive = 0;
	rq.primitiveCount = geo->GetPrimitiveCount();
	rq.primitiveType = geo->GetPrimitiveType();
	rq.frontFace = geo->GetFrontFaceWinding();
	return rq;
}
RenderRequest RenderRequest::Geometry3D(const Geometry* geo, int first, int count)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
	rq.is3D = true;
	rq.bufferData.ib = geo->GetIndices();
	rq.bufferData.vb = geo->GetVertices();
	rq.indexed = geo->GetIndices();
	rq.firstPrimitive = first;
	rq.primitiveCount = count;
	rq.primitiveType = geo->GetPrimitiveType();
	rq.frontFace = geo->GetFrontFaceWinding();
	return rq;
}
RenderRequest RenderRequest::Geometry2D(const Geometry* geo, int first, int count)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
	rq.is3D = false;
	rq.bufferData.ib = geo->GetIndices();
	rq.bufferData.vb = geo->GetVertices();
	rq.indexed = geo->GetIndices();
	rq.firstPrimitive = first;
	rq.primitiveCount = count;
	rq.primitiveType = geo->GetPrimitiveType();
	rq.frontFace = geo->GetFrontFaceWinding();
	return rq;
}

RendererNull::RendererNull(VideoDriver* driver) :
	m_RenderMode(ERenderMode::None),
	m_NormalizeNormals(true),
	m_DirtyFlags(0xFFFFFFFF), // Set all dirty flags at start
	m_Driver(driver)
{
	core::AttributeListBuilder alb;
	m_ParamIds.lighting = alb.AddAttribute("lighting", (float)video::ELightingFlag::Enabled);
	m_ParamIds.fogEnabled = alb.AddAttribute("fogEnabled", 1.0f);

	m_ParamIds.ambient = alb.AddAttribute("ambient", video::ColorF(0, 0, 0));
	m_ParamIds.time = alb.AddAttribute("time", 0.0f);

	for(int i = 0; i < m_MatrixTable.GetCount(); ++i)
		alb.AddAttribute(m_MatrixTable.CreateAttribute(i));

	for(int i = 0; i < 16; ++i)
		m_ParamIds.lights.PushBack(alb.AddAttribute("light" + core::StringConverter::ToString(i), math::Matrix4::ZERO));

	m_Params = m_BaseParams = alb.BuildAndReset();

	m_RenderStatistics = RenderStatistics::Instance();
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::SetPass(const Pass& pass, bool useOverwrite, ShaderParamSetCallback* paramSetCallback, void* userParam)
{
	SetDirty(Dirty_Pass);

	m_Pass = pass;
	m_ParamSetCallback = paramSetCallback;
	m_UserParam = userParam;
	m_UseOverwrite = useOverwrite;
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
	for(int i = 1; i < m_PipelineOverwrites.Size(); ++i)
		m_FinalOverwrite.Append(m_PipelineOverwrites[i]);
}

///////////////////////////////////////////////////////////////////////////

void RendererNull::AddLight(const LightData& light)
{
	if(m_Lights.Size() == GetMaxLightCount())
		throw core::InvalidOperationException("Too many lights");

	m_Lights.PushBack(light);
	SetDirty(Dirty_Lights);
}

void RendererNull::ClearLights()
{
	m_Lights.Clear();
	SetDirty(Dirty_Lights);
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
	default: throw core::GenericInvalidArgumentException("transform", "Unknown transform");
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

core::AttributeList RendererNull::GetBaseParams() const
{
	return m_BaseParams;
}

void RendererNull::SetParams(core::AttributeList attributes)
{
	if(m_Params != attributes) {
		auto p = m_Params;
		while(p.IsValid() && p != m_BaseParams)
			p = p.GetBase();
		if(!p.IsValid())
			throw core::GenericInvalidArgumentException("attributes", "UserParams must inherit from BaseParams.");
	}
	m_Params = attributes;
}

core::AttributeList RendererNull::GetParams() const
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
		throw core::GenericInvalidArgumentException("data.type", "Unknown data type");

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
