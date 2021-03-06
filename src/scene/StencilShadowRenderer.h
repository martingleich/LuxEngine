#ifndef INCLUDED_LUX_STENCIL_SHADOW_RENDERER_H
#define INCLUDED_LUX_STENCIL_SHADOW_RENDERER_H
#include "video/Renderer.h"
#include "video/Pass.h"
#include "video/VertexTypes.h"
#include "core/lxOrderedMap.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/RenderTarget.h"
#include "video/MaterialLibrary.h"

namespace lux
{
namespace scene
{

class StencilShadowRenderer
{
public:
	//! Contains the shadow volume generated by a mesh and a light
	struct ShadowVolume
	{
		core::Array<math::Vector3F> points;

		void AddQuad(
			const math::Vector3F& v1,
			const math::Vector3F& v2,
			const math::Vector3F& v1e,
			const math::Vector3F& v2e)
		{
			points.PushBack(v1);
			points.PushBack(v2);
			points.PushBack(v1e);

			points.PushBack(v2);
			points.PushBack(v2e);
			points.PushBack(v1e);
		}
	};

	//! Contains adjacence information for a mesh
	struct AdjacenceInfo
	{
		AdjacenceInfo() :
			geo(nullptr)
		{
		}

		int changeId;
		const video::Geometry* geo;

		struct Face
		{
			// Points of the face are 0,1,2
			// Edges of the face are 01, 12, 20
			// adj1 triangle, sharing 01
			// adj2 triangle, sharing 12
			// adj3 triangle, sharing 20
			// If there is no adjacent triangle it's set to the id of this triangle
			u32 adj[3];
			u16 points[3];
			math::Vector3F normal;
		};

		core::Array<Face> faces;
		core::Array<math::Vector3F> points;
	};

public:
	StencilShadowRenderer(video::Renderer* renderer, u32 stencilBitMask) :
		m_StencilBitMask(stencilBitMask),
		m_Renderer(renderer)
	{
		m_ShadowRenderPass.stencil = GetShadowStencilMode();
		m_ShadowRenderPass.zBufferFunc = video::EComparisonFunc::Always;
		m_ShadowRenderPass.zWriteEnabled = false;
		m_ShadowRenderPass.lighting = video::ELightingFlag::Disabled;
		m_ShadowRenderPass.fogEnabled = false;
		m_ShadowRenderPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
		m_ShadowRenderPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
		m_ShadowRenderPass.alpha.blendOperator = video::EBlendOperator::Add;
		m_ShadowRenderPass.shader = video::ShaderFactory::Instance()->
			GetFixedFunctionShader(video::FixedFunctionParameters::VertexColorOnly());

		m_Silhouette.colorMask = 0;
		m_Silhouette.shading = video::EShading::Flat;
		if(m_UseZFail) {
			m_Silhouette.stencil.zFail = video::EStencilOperator::Increment;
			m_Silhouette.stencil.zFailCCW = video::EStencilOperator::Decrement;
		} else {
			m_Silhouette.stencil.pass = video::EStencilOperator::Decrement;
			m_Silhouette.stencil.passCCW = video::EStencilOperator::Increment;
		}
		m_Silhouette.stencil.writeMask = m_StencilBitMask;
		m_Silhouette.stencil.readMask = m_StencilBitMask;
		m_Silhouette.zWriteEnabled = false;
		m_Silhouette.culling = video::EFaceSide::None;
		m_Silhouette.lighting = video::ELightingFlag::Disabled;
		m_Silhouette.fogEnabled = false;
		m_Silhouette.polygonOffset = -100.0f;
		m_Silhouette.shader = video::ShaderFactory::Instance()->
			GetFixedFunctionShader(video::FixedFunctionParameters::Unlit({}, {}, false));
	}

	// Begin a new silhouette rendering
	void Begin(const math::Vector3F& camPos, const math::Vector3F& camDir)
	{
		m_CamPos = camPos;
		m_CamDir = camDir;
	}

	//! Add a silhouette to the stencil buffer
	void AddSilhouette(const math::Transformation& transform, const video::Mesh* mesh, math::Vector3F& lightPos, bool isInfiniteLight)
	{
		ShadowVolume shadowVolume;
		GenerateVolume(mesh, lightPos, isInfiniteLight, shadowVolume);

		if(!shadowVolume.points.IsEmpty()) {
			m_Renderer->SetTransform(video::ETransform::World, transform.ToMatrix());
			m_Renderer->SendPassSettings(m_Silhouette);
			m_Renderer->Draw(video::RenderRequest::FromMemory(
				video::EPrimitiveType::Triangles,
				shadowVolume.points.Size() / 3,
				shadowVolume.points.Data(),
				shadowVolume.points.Size(),
				video::VertexFormat::POS_ONLY));
		}
	}

	void GenerateVolume(const video::Mesh* mesh, const math::Vector3F& lightPos, bool isInfiniteLight,
		ShadowVolume& outVolume)
	{
		const AdjacenceInfo& adjInfo = GetAdjacenceInfo(mesh);

		// Generate facing information
		auto faceCount = adjInfo.faces.Size();
		m_FacingData.Resize(faceCount);
		for(int i = 0; i < faceCount; ++i) {
			if(isInfiniteLight)
				m_FacingData[i] = (adjInfo.faces[i].normal.Dot(lightPos) < 0);
			else
				m_FacingData[i] = (adjInfo.faces[i].normal.Dot(lightPos - adjInfo.points[adjInfo.faces[i].points[0]]) < 0);
		}

		auto extend = [&](const math::Vector3F& v) {
			if(isInfiniteLight)
				return v + lightPos*100.0f;
			else
				return v + (v - lightPos).Normal()*100.0f;
		};

		// Foreach forward facing face, check if adjacent face is backward facing, then add edge to contour
		for(int i = 0; i < faceCount; ++i) {
			if(!m_FacingData[i])
				continue;

			const auto& face = adjInfo.faces[i];
			u32 adj0 = face.adj[0];
			u32 adj1 = face.adj[1];
			u32 adj2 = face.adj[2];

			math::Vector3F v1, v2, v3;
			math::Vector3F v1e, v2e, v3e;

			v1 = adjInfo.points[face.points[0]];
			v2 = adjInfo.points[face.points[1]];
			v3 = adjInfo.points[face.points[2]];

			if(m_UseZFail) {
				outVolume.points.PushBack(v2);
				outVolume.points.PushBack(v1);
				outVolume.points.PushBack(v3);

				outVolume.points.PushBack(extend(v3));
				outVolume.points.PushBack(extend(v1));
				outVolume.points.PushBack(extend(v2));
			}

			if(m_FacingData[i] != m_FacingData[adj0] || (u32)i == adj0) {
				outVolume.AddQuad(v1, v2, extend(v1), extend(v2));
			}

			if(m_FacingData[i] != m_FacingData[adj1] || (u32)i == adj1) {
				outVolume.AddQuad(v2, v3, extend(v2), extend(v3));
			}

			if(m_FacingData[i] != m_FacingData[adj2] || (u32)i == adj2) {
				outVolume.AddQuad(v3, v1, extend(v3), extend(v1));
			}
		}
	}

	// After this call the shadow areas are marked in the stencil buffer
	void End()
	{
	}

	// Draw with this stencil mode to only draw in shadows
	video::StencilMode GetShadowStencilMode()
	{
		video::StencilMode sm;
		sm.test = video::EComparisonFunc::NotEqual;
		sm.ref = 0;
		sm.readMask = m_StencilBitMask;
		return sm;
	}

	// Draw with this stencil mode to only draw in illumniated areas
	video::StencilMode GetIllumniatedStencilMode()
	{
		video::StencilMode sm;
		sm.test = video::EComparisonFunc::Equal;
		sm.ref = 0;
		sm.readMask = m_StencilBitMask;
		return sm;
	}

	//! Fill all shadow areas with a given color
	void DisplayShadow(video::Color color = video::Color(0, 0, 0, 100))
	{
		auto size = m_Renderer->GetRenderTarget().GetSize();
		const video::Vertex2D points[4] =
		{
			video::Vertex2D(0.0f, 0.0f, color),
			video::Vertex2D((float)size.width, 0.0f, color),
			video::Vertex2D(0.0f, (float)size.height, color),
			video::Vertex2D((float)size.width, (float)size.height, color)
		};

		m_Renderer->SendPassSettings(m_ShadowRenderPass);
		m_Renderer->Draw(video::RenderRequest::FromMemory(
			video::EPrimitiveType::TriangleStrip,
			2, &points, 4,
			video::VertexFormat::STANDARD_2D));
	}

private:
	const AdjacenceInfo& GetAdjacenceInfo(const video::Mesh* mesh)
	{
		AdjacenceInfo& info = m_AdjacenceInfo[mesh];
		if(info.geo == nullptr || info.geo != mesh->GetGeometry() || info.changeId != mesh->GetGeometry()->GetChangeId())
			GenerateAdjacenceInfo(mesh, info);
		return info;
	}

	void GenerateAdjacenceInfo(const video::Mesh* mesh, AdjacenceInfo& info)
	{
		info.geo = mesh->GetGeometry();
		info.changeId = info.geo->GetChangeId();
		info.faces.Clear();
		info.points.Clear();

		// Copy mesh
		auto vertexCount = info.geo->GetVertexCount();
		info.points.Reserve(vertexCount);
		auto vb = info.geo->GetVertices();
		auto poff = info.geo->GetVertexFormat().GetElement(video::VertexElement::EUsage::Position).GetOffset();
		for(int i = 0; i < vertexCount; ++i) {
			auto pos = *(math::Vector3F*)((u8*)vb->Pointer_c(i, 1) + poff);
			info.points.PushBack(pos);
		}

		// TEMP: Assume triangle list
		lxAssert(info.geo->GetPrimitiveType() == video::EPrimitiveType::Triangles);
		lxAssert(info.geo->GetIndexFormat() == video::EIndexFormat::Bit16);

		auto ib = info.geo->GetIndices();
		auto faceCount = info.geo->GetPrimitiveCount();
		info.faces.Resize(faceCount);
		for(int i = 0; i < faceCount; ++i) {
			ib->GetIndices(info.faces[i].points, 3, 3 * i);
			auto pa = info.points[info.faces[i].points[0]];
			auto pb = info.points[info.faces[i].points[1]];
			auto pc = info.points[info.faces[i].points[2]];
			info.faces[i].normal = (pb - pa).Cross(pc - pa);
		}

		// Generate adjacence info
		for(int i = 0; i < faceCount; ++i) {
			for(int e = 0; e < 3; ++e) {
				auto v1 = info.points[info.faces[i].points[e]];
				auto v2 = info.points[info.faces[i].points[(e + 1) % 3]];

				int j;
				for(j = 0; j < faceCount; ++j) {
					if(i != j) {
						bool cnt1 = false;
						bool cnt2 = false;
						for(int e2 = 0; e2 < 3; ++e2) {
							auto v = info.points[info.faces[j].points[e2]];
							if(math::IsEqual(v1, v))
								cnt1 = true;
							if(math::IsEqual(v2, v))
								cnt2 = true;
						}
						if(cnt1 && cnt2)
							break;
					}
				}
				if(j >= faceCount) { // Not adjacent edges
					info.faces[i].adj[e] = i;
				} else {
					info.faces[i].adj[e] = j;
				}
			}
		}
	}

private:
	bool m_UseZFail = true;

	video::Pass m_ShadowRenderPass;

	video::Pass m_Silhouette;

	u32 m_StencilBitMask;
	math::Vector3F m_CamPos;
	math::Vector3F m_CamDir;

	core::OrderedMap<const video::Mesh*, AdjacenceInfo> m_AdjacenceInfo;
	core::Array<ShadowVolume> m_Volumes;
	core::Array<bool> m_FacingData;

	video::Renderer* m_Renderer;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_STENCIL_SHADOW_RENDERER_H