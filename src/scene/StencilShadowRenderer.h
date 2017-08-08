#ifndef INCLUDED_STENCIL_SHADOW_RENDERER_H
#define INCLUDED_STENCIL_SHADOW_RENDERER_H
#include "video/Renderer.h"
#include "video/Pass.h"
#include "video/VertexTypes.h"
#include "core/lxOrderedMap.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/RenderTarget.h"

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

		u32 changeId;
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

		video::Pass p;
		p.colorMask = 0;
		p.gouraudShading = false;
		if(m_UseZFail) {
			p.stencil.zFail = video::EStencilOperator::Increment;
			p.stencil.zFailCCW = video::EStencilOperator::Decrement;
		} else {
			p.stencil.pass = video::EStencilOperator::Decrement;
			p.stencil.passCCW = video::EStencilOperator::Increment;
		}
		p.stencil.writeMask = m_StencilBitMask;
		p.stencil.readMask = m_StencilBitMask;
		p.zWriteEnabled = false;
		p.backfaceCulling = false;
		p.frontfaceCulling = false;
		p.lighting = video::ELighting::Disabled;
		p.fogEnabled = false;
		p.polygonOffset = -100.0f;
		m_Renderer->SetPass(p);

		m_Renderer->SetTransform(video::ETransform::World, transform.ToMatrix());
		if(!shadowVolume.points.IsEmpty()) {
			m_Renderer->DrawPrimitiveList(video::EPrimitiveType::Triangles,
				shadowVolume.points.Size() / 3,
				shadowVolume.points.Data(),
				shadowVolume.points.Size(),
				video::VertexFormat::POS_ONLY,
				true);
		}
	}

	void GenerateVolume(const video::Mesh* mesh, const math::Vector3F& lightPos, bool isInfiniteLight,
		ShadowVolume& outVolume)
	{
		const AdjacenceInfo& adjInfo = GetAdjacenceInfo(mesh);

		// Generate facing information
		const u32 faceCount = (u32)adjInfo.faces.Size();
		m_FacingData.Resize(faceCount);
		for(u32 i = 0; i < faceCount; ++i) {
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
		for(u32 i = 0; i < faceCount; ++i) {
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

			if(m_FacingData[i] != m_FacingData[adj0] || i == adj0) {
				outVolume.AddQuad(v1, v2, extend(v1), extend(v2));
			}

			if(m_FacingData[i] != m_FacingData[adj1] || i == adj1) {
				outVolume.AddQuad(v2, v3, extend(v2), extend(v3));
			}

			if(m_FacingData[i] != m_FacingData[adj2] || i == adj2) {
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

		video::Pass p;
		p.stencil = GetShadowStencilMode();
		p.zBufferFunc = video::EComparisonFunc::Always;
		p.zWriteEnabled = false;
		p.lighting = video::ELighting::Disabled;
		p.isTransparent = true;
		p.fogEnabled = false;
		p.useVertexColor = true;
		p.alphaOperator = video::EBlendOperator::Add;
		p.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
		p.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		m_Renderer->SetPass(p);
		m_Renderer->DrawPrimitiveList(video::EPrimitiveType::TriangleStrip, 2, &points, 4, video::VertexFormat::STANDARD_2D, false);
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
		auto poff = info.geo->GetVertexFormat().GetElement(video::VertexElement::EUsage::Position).offset;
		for(u32 i = 0; i < vertexCount; ++i) {
			auto pos = *(math::Vector3F*)((u8*)vb->Pointer_c(i, 1) + poff);
			info.points.PushBack(pos);
		}

		// TEMP: Assume triangle list
		lxAssert(info.geo->GetPrimitiveType() == video::EPrimitiveType::Triangles);
		lxAssert(info.geo->GetIndexType() == video::EIndexFormat::Bit16);

		auto ib = info.geo->GetIndices();
		auto faceCount = info.geo->GetPrimitiveCount();
		info.faces.Resize(faceCount);
		for(u32 i = 0; i < faceCount; ++i) {
			ib->GetIndices(info.faces[i].points, 3, 3 * i);
			auto pa = info.points[info.faces[i].points[0]];
			auto pb = info.points[info.faces[i].points[1]];
			auto pc = info.points[info.faces[i].points[2]];
			info.faces[i].normal = (pb - pa).Cross(pc - pa);
		}

		// Generate adjacence info
		for(u32 i = 0; i < faceCount; ++i) {
			for(u32 e = 0; e < 3; ++e) {
				auto v1 = info.points[info.faces[i].points[e]];
				auto v2 = info.points[info.faces[i].points[(e + 1) % 3]];

				u32 j;
				for(j = 0; j < faceCount; ++j) {
					if(i != j) {
						bool cnt1 = false;
						bool cnt2 = false;
						for(u32 e2 = 0; e2 < 3; ++e2) {
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

#endif // #ifndef INCLUDED_STENCIL_SHADOW_RENDERER_H