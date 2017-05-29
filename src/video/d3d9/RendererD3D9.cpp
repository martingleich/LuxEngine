#ifdef LUX_COMPILE_WITH_D3D9
#include "RendererD3D9.h"

#include "video/TextureLayer.h"
#include "video/BaseTexture.h"
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/D3D9Exception.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"
#include "video/SubMesh.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexTypes.h"
#include "video/Shader.h"
#include "core/Logger.h"

namespace lux
{
namespace video
{

class ParamListAccessNull : public RenderSettings::ParamListAccess
{
public:
	ParamListAccessNull(RendererNull* r) :
		m_Renderer(r)
	{
	}

	core::PackageParam operator[](u32 id) const
	{
		return m_Renderer->GetParam(id);
	}

private:
	RendererNull* m_Renderer;
};

RendererD3D9::RendererD3D9(VideoDriverD3D9* driver) :
	RendererNull(driver),
	m_Device((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_State((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_Driver(driver),
	m_MaterialRenderer(nullptr)
{
	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_CurrentRendertarget = m_BackbufferTarget;
}

void RendererD3D9::CleanUp()
{
	m_InvalidMaterial.Reset();
	m_Material.Reset();
}

void RendererD3D9::BeginScene(bool clearColor, bool clearZ,
	video::Color color, float z)
{
	if((m_CurrentRendertarget.GetTexture() == nullptr) && m_CurrentRendertarget != m_BackbufferTarget) {
		log::Warning("The current rendertarget texture was destroyed, fallback to normal backbuffer.");
		SetRenderTarget(nullptr);
	}

	u32 flags = 0;
	if(clearColor)
		flags = D3DCLEAR_TARGET;
	if(clearZ)
		flags |= D3DCLEAR_ZBUFFER;

	HRESULT hr = S_OK;

	if(flags) {
		const D3DCOLOR d3dClear = (u32)color;
		if(FAILED(hr = m_Device->Clear(
			0, nullptr,
			flags,
			d3dClear, z, 0))) {
			throw core::D3D9Exception(hr);
		}
	}

	if(FAILED(hr) || FAILED(hr = m_Device->BeginScene())) {
		if(hr == D3DERR_INVALIDCALL)
			throw core::Exception("Scene was already started");
		else
			throw core::D3D9Exception(hr);
	}
}

void RendererD3D9::EndScene()
{
	HRESULT hr;
	if(FAILED(hr = m_Device->EndScene())) {
		if(hr == D3DERR_INVALIDCALL)
			throw core::Exception("Scene was already started");
		else
			throw core::D3D9Exception(hr);
	}

	m_RenderStatistics->RegisterFrame();
}

bool RendererD3D9::Present()
{
	HRESULT hr = m_Device->Present(NULL, NULL, NULL, NULL);

	return SUCCEEDED(hr);
}

void RendererD3D9::SetRenderTarget(const RenderTarget& target)
{
	if(!target.IsBackbuffer()) {
		if(!target.GetTexture()->IsRendertarget())
			throw core::InvalidArgumentException("target", "Must be a rendertarget texture");

		// Same texture as current target -> Abort
		if(target.GetTexture() == m_CurrentRendertarget.GetTexture())
			return;
	} else {
		// Tries to set to back buffer and backbuffer already loaded -> Abort
		if(m_CurrentRendertarget == m_BackbufferTarget)
			return;
	}

	RendertargetD3D9 newRendertarget;
	if(target.IsBackbuffer())
		newRendertarget = m_BackbufferTarget;
	else
		newRendertarget = target;

	// Generate depth buffer via video driver
	IDirect3DSurface9* depthStencil = m_Driver->GetD3D9MatchingDepthBuffer(newRendertarget.GetSurface());
	if(depthStencil == nullptr)
		throw core::Exception("Can't find matching depth buffer for rendertarget.");

	HRESULT  hr;
	if(FAILED(hr = m_Device->SetRenderTarget(0, newRendertarget.GetSurface())))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_Device->SetDepthStencilSurface(depthStencil))) {
		// Restore old target and fail
		m_Device->SetRenderTarget(0, m_CurrentRendertarget.GetSurface());
		throw core::D3D9Exception(hr);
	}

	m_State.SetRenderTargetTexture(target.GetTexture());

	m_CurrentRendertarget = newRendertarget;
}

const RenderTarget& RendererD3D9::GetRenderTarget()
{
	return m_CurrentRendertarget;
}

///////////////////////////////////////////////////////////////////////////

size_t RendererD3D9::GetMaxLightCount() const
{
	return m_Driver->GetDeviceCapability(EDriverCaps::MaxLights);
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::DrawPrimitiveList(
	EPrimitiveType primitiveType, u32 primitiveCount,
	const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
	const void* indexData, EIndexFormat indexType,
	bool is3D)
{
	if(primitiveCount == 0)
		return;

	SetupRendering(is3D ? ERenderMode::Mode3D : ERenderMode::Mode2D);

	SetVertexFormat(vertexFormat);
	u32 stride = vertexFormat.GetStride(0);

	D3DPRIMITIVETYPE d3dPrimitiveType = GetD3DPrimitiveType(primitiveType);

	HRESULT hr = E_FAIL;
	if(!vertexData && !indexData) {
		// Draw indexed from memory
		StrongRef<BufferManagerD3D9> d3d9Manager = m_Driver->GetBufferManager();

		BufferManagerD3D9::VertexStream vs;
		BufferManagerD3D9::IndexStream is;

		u32 vertexOffset = 0;
		u32 indexOffset = 0;
		if(d3d9Manager->GetVertexStream(0, vs))
			vertexOffset = vs.offset;

		if(d3d9Manager->GetIndexStream(is))
			indexOffset = is.offset;

		hr = m_Device->DrawIndexedPrimitive(d3dPrimitiveType, 0, 0, vertexCount, indexOffset, primitiveCount);
	}
	if(vertexData && indexData) {
		// Draw indexed from memory
		D3DFORMAT d3dIndexFormat = GetD3DIndexFormat(indexType);
		hr = m_Device->DrawIndexedPrimitiveUP(d3dPrimitiveType, 0, vertexCount, primitiveCount,
			indexData, d3dIndexFormat, vertexData, stride);
	}
	if(vertexData) {
		// Draw from memory
		hr = m_Device->DrawPrimitiveUP(d3dPrimitiveType, primitiveCount, vertexData, stride);
	}

	if(FAILED(hr)) {
		log::Error("Error while drawing.");
		return;
	}

	m_RenderStatistics->AddPrimitves(primitiveCount);
}

void RendererD3D9::DrawSubMesh(const SubMesh* subMesh, u32 primitiveCount, bool is3D)
{
	LX_CHECK_NULL_ARG(subMesh);

	if(primitiveCount == 0xFFFFFFFF)
		primitiveCount = subMesh->GetPrimitveCount();

	if(primitiveCount == 0)
		return;

	auto bufferManager = m_Driver->GetBufferManager();

	bufferManager->EnableBuffer(subMesh->GetVertices());
	bufferManager->EnableBuffer(subMesh->GetIndices());

	BufferManagerD3D9* d3d9Manager = bufferManager.As<BufferManagerD3D9>();

	BufferManagerD3D9::VertexStream vs;
	BufferManagerD3D9::IndexStream is;
	d3d9Manager->GetVertexStream(0, vs);
	d3d9Manager->GetIndexStream(is);

	const EPrimitiveType pt = subMesh->GetPrimitiveType();
	const u32 vertexCount = subMesh->GetVertexCount();
	const VertexFormat& vertexFormat = subMesh->GetVertexFormat();
	const EIndexFormat indexType = subMesh->GetIndices() ?
		subMesh->GetIndexType() :
		EIndexFormat::Bit16;

	DrawPrimitiveList(pt,
		primitiveCount,
		vs.data,
		vertexCount,
		vertexFormat,
		is.data,
		indexType,
		is3D);
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::SetupRendering(ERenderMode mode)
{
	// Generate pipeline
	if(IsDirty(Dirty_MaterialRenderer) || IsDirty(Dirty_PipelineOverwrites)) {
		bool oldLighting = m_Pipeline.lighting;
		bool oldFogging = m_Pipeline.fogEnabled;
		float oldPolyOffset = m_Pipeline.polygonOffset;

		m_Pipeline = m_Material->GetRenderer()->GetPipeline();

		// Apply overwrites
		for(auto it = m_PipelineOverwrites.First(); it != m_PipelineOverwrites.End(); ++it)
			it->Apply(m_Pipeline);

		if(m_Pipeline.fogEnabled != oldFogging)
			SetDirty(Dirty_Fog);

		if(m_Pipeline.lighting != oldLighting)
			SetDirty(Dirty_Lights);

		if(m_Pipeline.polygonOffset != oldPolyOffset)
			SetDirty(Dirty_PolygonOffset);

		SetDirty(Dirty_Pipeline);
		ClearDirty(Dirty_PipelineOverwrites);
	}

	SwitchRenderMode(mode);

	LoadSettings();

	ClearAllDirty();
}

void RendererD3D9::SwitchRenderMode(ERenderMode mode)
{
	// Leave the old mode
	switch(m_RenderMode) {
	case ERenderMode::None: /*nothing to do*/break;
	case ERenderMode::Mode2D: LeaveRenderMode2D(); break;
	case ERenderMode::Mode3D: LeaveRenderMode3D(); break;
	}

	// Enter the new mode
	switch(mode) {
	case ERenderMode::None: /*nothing to do*/break;
	case ERenderMode::Mode2D: EnterRenderMode2D(); break;
	case ERenderMode::Mode3D: EnterRenderMode3D(); break;
	}

	m_RenderMode = mode;
}

void RendererD3D9::EnterRenderMode3D()
{
}

void RendererD3D9::LeaveRenderMode3D()
{
}

void RendererD3D9::EnterRenderMode2D()
{
	SetDirty(Dirty_Lights);
	SetDirty(Dirty_Fog);
}

void RendererD3D9::LeaveRenderMode2D()
{
	SetDirty(Dirty_Lights);
	SetDirty(Dirty_Fog);
}

void RendererD3D9::LoadSettings()
{
	// Collect rendersettings
	ParamListAccessNull listAccess(this);
	RenderSettings settings(
		m_Material->GetRenderer(),
		m_Material,
		m_Pipeline,
		listAccess);

	LoadTransforms(settings);

	LoadFogSettings(settings);

	LoadLightSettings(settings);

	// Begin the new renderer
	auto newRenderer = settings.renderer;
	if(IsDirty(Dirty_MaterialRenderer) || IsDirty(Dirty_Material)) {
		if(m_MaterialRenderer)
			m_MaterialRenderer->End(m_State);

		if(newRenderer)
			newRenderer->Begin(settings, m_State);

		m_MaterialRenderer = newRenderer;

		// Enable the d3d material
		m_State.SetD3DMaterial(
			GetParam(m_ParamId.ambient),
			settings.pipeline,
			settings.material);
	}

	if(m_MaterialRenderer->GetShader())
		m_MaterialRenderer->GetShader()->LoadSettings(settings);

	// Enable the pipeline
	if(IsDirty(Dirty_Pipeline)) {
		m_State.EnablePipeline(settings.pipeline);
		ClearDirty(Dirty_Pipeline);
	}
}

void RendererD3D9::LoadTransforms(const RenderSettings& settings)
{
	// Load transforms
	if(m_RenderMode == ERenderMode::Mode3D) {
		if(IsDirty(Dirty_PolygonOffset)) {
			math::matrix4 projCopy = m_TransformProj; // The userset projection matrix
			if(m_Pipeline.polygonOffset) {
				const u32 zBits = m_Driver->GetConfig().zBits;
				const u32 values = 1 << zBits;
				const float min = 1.0f / values;
				projCopy.AddTranslation(math::vector3f(0.0f, 0.0f, -min *m_Pipeline.polygonOffset));
			}
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, projCopy);
			SetDirty(Dirty_Transform);
		}
	} else if(m_RenderMode == ERenderMode::Mode2D) {
		if(IsDirty(Dirty_Rendertarget)) {
			auto ssize = GetRenderTarget().GetSize();
			math::matrix4 view = math::matrix4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				-(float)ssize.width / 2 - 0.5f, (float)ssize.height / 2 - 0.5f, 0.0f, 1.0f);

			math::matrix4 proj = math::matrix4(
				2.0f / ssize.width, 0.0f, 0.0f, 0.0f,
				0.0f, 2.0f / ssize.height, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m_MatrixTable.SetMatrix(MatrixTable::MAT_VIEW, view);
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, proj);

			SetDirty(Dirty_Transform);
		}
	}

	if(IsDirty(Dirty_Transform)) {
		if(settings.renderer->GetShader() == nullptr) {
			m_State.SetTransform(D3DTS_PROJECTION, m_MatrixTable.GetMatrix(MatrixTable::MAT_PROJ));
			m_State.SetTransform(D3DTS_WORLD, m_MatrixTable.GetMatrix(MatrixTable::MAT_WORLD));
			m_State.SetTransform(D3DTS_VIEW, m_MatrixTable.GetMatrix(MatrixTable::MAT_VIEW));
		}

		ClearDirty(Dirty_Transform);
	}
}

void RendererD3D9::LoadFogSettings(const RenderSettings& settings)
{
	if(IsDirty(Dirty_Fog)) {
		bool useFog = settings.pipeline.fogEnabled && m_Fog.isActive;
		if(m_RenderMode == ERenderMode::Mode2D)
			useFog = false;

		bool useFixedFog = useFog;
		if(settings.renderer->GetShader() != nullptr)
			useFixedFog = false;

		math::vector3f fogInfo;
		fogInfo.x = useFog;
		fogInfo.y =
			m_Fog.type == FogData::EType::Linear ? 1.0f :
			m_Fog.type == FogData::EType::Exp ? 2.0f :
			m_Fog.type == FogData::EType::ExpSq ? 3.0f : 1.0f;
		fogInfo.z = 0;
		GetParamInternal(m_ParamId.fogInfo) = fogInfo;

		if(useFog) {
			GetParamInternal(m_ParamId.fogColor) = m_Fog.color;
			GetParamInternal(m_ParamId.fogRange) = math::vector3f(
				m_Fog.start,
				m_Fog.end,
				m_Fog.density);
		}

		// Only use the fixed pipeline if there is no shader
		m_State.SetFog(useFixedFog, m_Fog);
		ClearDirty(Dirty_Fog);
	}
}

void RendererD3D9::LoadLightSettings(const RenderSettings& settings)
{
	if(IsDirty(Dirty_Lights)) {
		bool useLights = settings.pipeline.lighting;
		if(m_RenderMode == ERenderMode::Mode2D)
			useLights = false;

		bool useFixedLights = useLights;
		if(settings.renderer->GetShader() != nullptr)
			useFixedLights = false;

		m_State.ClearLights(useFixedLights);
		GetParam(m_ParamId.lighting) = settings.pipeline.lighting ? 1.0f : 0.0f;
		if(useLights) {
			// Only use the fixed pipeline if there is no shader
			if(!settings.renderer->GetShader()) {
				// Enable fixed pipeline lights
				for(auto it = m_Lights.First(); it != m_Lights.End(); ++it)
					m_State.EnableLight(*it);
			} else {
				// Generate light matrices and set as param.
				for(size_t i = 0; i < m_Lights.Size(); ++i)
					GetParamInternal(m_ParamId.lights[i]) = GenerateLightMatrix(m_Lights[i], true);
			}
		}
		ClearDirty(Dirty_Lights);
	}

}

void RendererD3D9::SetVertexFormat(const VertexFormat& format)
{
	if(format == m_VertexFormat)
		return;

	HRESULT hr;

	auto decl = m_Driver->GetD3D9VertexDeclaration(format);
	if(FAILED(hr = m_Device->SetVertexDeclaration(decl)))
		throw core::D3D9Exception(hr);

	m_VertexFormat = format;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
