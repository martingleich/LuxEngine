#include "RendererNull.h"
#include "core/Logger.h"
#include "video/RenderTarget.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

RenderRequest RenderRequest::FromGeometry(const Geometry* geo)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
	rq.bufferData.ib = geo->GetIndices();
	rq.bufferData.vb = geo->GetVertices();
	rq.indexed = geo->GetIndices();
	rq.firstPrimitive = 0;
	rq.primitiveCount = geo->GetPrimitiveCount();
	rq.primitiveType = geo->GetPrimitiveType();
	rq.frontFace = geo->GetFrontFaceWinding();
	return rq;
}
RenderRequest RenderRequest::FromGeometry(const Geometry* geo, int first, int count)
{
	LX_CHECK_NULL_ARG(geo);

	RenderRequest rq;
	rq.userPointer = false;
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

	m_Params = m_BaseParams = alb.BuildAndReset();

	m_RenderStatistics = RenderStatistics::Instance();
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
	for(int i = 1; i < m_PipelineOverwrites.Size(); ++i)
		m_FinalOverwrite.Append(m_PipelineOverwrites[i]);
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
	case ETransform::World: return m_MatrixTable.GetMatrix(MatrixTable::MAT_WORLD);
	case ETransform::View: return m_MatrixTable.GetMatrix(MatrixTable::MAT_VIEW);
	case ETransform::Projection: return m_MatrixTable.GetMatrix(MatrixTable::MAT_PROJ);
	default: throw core::GenericInvalidArgumentException("transform", "Unknown transform");
	}
}

void RendererNull::SetNormalizeNormals(bool normalize, NormalizeNormalsToken* token)
{
	if(token && !token->renderer) {
		token->renderer = this;
		token->prev = m_NormalizeNormals;
	}
	m_NormalizeNormals = normalize;
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

} // namespace video
} // namespace lux
