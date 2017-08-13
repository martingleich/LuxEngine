#ifdef LUX_COMPILE_WITH_D3D9
#include "RendererD3D9.h"

#include "video/TextureLayer.h"
#include "video/BaseTexture.h"
#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/D3D9Exception.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexTypes.h"
#include "video/Shader.h"
#include "core/Logger.h"

namespace lux
{
namespace video
{

RendererD3D9::RendererD3D9(VideoDriverD3D9* driver) :
	RendererNull(driver),
	m_Device((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_DeviceState((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_Driver(driver),
	m_MaterialRenderer(nullptr)
{
	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_ScissorRect.Set(0, 0, m_BackbufferTarget.GetSize().width, m_BackbufferTarget.GetSize().height);
	m_CurrentRendertarget = m_BackbufferTarget;
}

void RendererD3D9::CleanUp()
{
	m_InvalidMaterial.Reset();
	m_Material.Reset();
}

void RendererD3D9::BeginScene(
	bool clearColor, bool clearZ, bool clearStencil,
	video::Color color, float z, u32 stencil)
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
	if(clearStencil && m_Driver->GetConfig().zsFormat.sBits != 0)
		flags |= D3DCLEAR_STENCIL;

	HRESULT hr = S_OK;

	if(flags) {
		const D3DCOLOR d3dClear = (u32)color;
		if(FAILED(hr = m_Device->Clear(
			0, nullptr,
			flags,
			d3dClear, z, stencil))) {
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

void RendererD3D9::ClearStencil(u32 value)
{
	u32 flags = 0;
	if(m_Driver->GetConfig().zsFormat.sBits != 0)
		flags |= D3DCLEAR_STENCIL;
	HRESULT hr;
	if(FAILED(hr = m_Device->Clear(
		0, nullptr,
		flags,
		0, 1.0f, value))) {
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

	// Reset scissor rectangle
	SetScissorRect(math::RectU(0, 0, target.GetSize().width, target.GetSize().height));

	m_DeviceState.SetRenderTargetTexture(target.GetTexture());

	m_CurrentRendertarget = newRendertarget;
}

const RenderTarget& RendererD3D9::GetRenderTarget()
{
	return m_CurrentRendertarget;
}

void RendererD3D9::SetScissorRect(const math::RectU& rect, ScissorRectToken* token)
{
	auto bufferSize = GetRenderTarget().GetSize();
	math::RectU fittedRect = rect;
	fittedRect.FitInto(math::RectU(0, 0, bufferSize.width, bufferSize.height));
	if(!fittedRect.IsValid())
		throw core::InvalidArgumentException("rect", "Rectangle is invalid");

	if(fittedRect.IsEmpty() || rect.GetSize() == GetRenderTarget().GetSize()) {
		m_DeviceState.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	} else {
		RECT r = {
			(LONG)fittedRect.left,
			(LONG)fittedRect.top,
			(LONG)fittedRect.right,
			(LONG)fittedRect.bottom};
		HRESULT hr = m_Device->SetScissorRect(&r);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
		m_DeviceState.SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	}

	if(token) {
		lxAssert(token->renderer == nullptr || token->renderer == this);
		if(token->renderer == nullptr) { // If the token was already used don't change it's values
			token->renderer = this;
			token->prevRect = m_ScissorRect;
		}
	}

	m_ScissorRect = fittedRect;
}

const math::RectU& RendererD3D9::GetScissorRect() const
{
	return m_ScissorRect;
}

///////////////////////////////////////////////////////////////////////////

size_t RendererD3D9::GetMaxLightCount() const
{
	return m_Driver->GetDeviceCapability(EDriverCaps::MaxLights);
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::DrawPrimitiveList(
	EPrimitiveType primitiveType, u32 firstPrimitive, u32 primitiveCount,
	const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
	const void* indexData, EIndexFormat indexType,
	bool is3D, bool user)
{
	if(primitiveCount == 0)
		return;

	SwitchRenderMode(is3D ? ERenderMode::Mode3D : ERenderMode::Mode2D);
	SetVertexFormat(vertexFormat);
	u32 stride = vertexFormat.GetStride(0);

	D3DFORMAT d3dIndexFormat = GetD3DIndexFormat(indexType);
	D3DPRIMITIVETYPE d3dPrimitiveType = GetD3DPrimitiveType(primitiveType);

	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	if(!user) {
		if(indexData) {
			// Indexed from stream
			StrongRef<BufferManagerD3D9> d3d9Manager = m_Driver->GetBufferManager();

			BufferManagerD3D9::VertexStream vs;
			BufferManagerD3D9::IndexStream is;

			if(d3d9Manager->GetVertexStream(0, vs))
				vertexOffset = vs.offset;

			if(d3d9Manager->GetIndexStream(is))
				indexOffset = is.offset;
		} else {
			// Not indexed from stream
			StrongRef<BufferManagerD3D9> d3d9Manager = m_Driver->GetBufferManager();

			BufferManagerD3D9::VertexStream vs;
			if(d3d9Manager->GetVertexStream(0, vs))
				vertexOffset = vs.offset;
		}
	}

	if(indexData)
		indexOffset += video::GetPointCount(primitiveType, firstPrimitive);
	else
		vertexOffset += video::GetPointCount(primitiveType, firstPrimitive);

	size_t passCount = m_UseMaterial ? m_Material->GetRenderer()->GetPassCount() : 1;
	for(size_t i = 0; i < passCount; ++i) {
		SetupRendering(i);

		HRESULT hr = E_FAIL;
		if(!user) {
			if(indexData)
				hr = m_Device->DrawIndexedPrimitive(d3dPrimitiveType, 0, 0, vertexCount, indexOffset, primitiveCount);
			else
				hr = m_Device->DrawPrimitive(d3dPrimitiveType, vertexOffset, primitiveCount);
		} else {
			if(indexData) // Indexed from memory
				hr = m_Device->DrawIndexedPrimitiveUP(d3dPrimitiveType, 0, vertexCount, primitiveCount,
					indexData, d3dIndexFormat, vertexData, stride);
			else // Not indexed from memory
				hr = m_Device->DrawPrimitiveUP(d3dPrimitiveType, primitiveCount, vertexData, stride);
		}
		if(FAILED(hr)) {
			log::Error("Error while drawing.");
			return;
		}
	}

	m_RenderStatistics->AddPrimitives(primitiveCount);
}

void RendererD3D9::DrawGeometry(const Geometry* geo, u32 firstPrimitive, u32 primitiveCount, bool is3D)
{
	LX_CHECK_NULL_ARG(geo);

	if(primitiveCount == 0xFFFFFFFF)
		primitiveCount = geo->GetPrimitiveCount();

	if(primitiveCount == 0)
		return;

	auto bufferManager = m_Driver->GetBufferManager();

	bufferManager->EnableBuffer(geo->GetVertices());
	bufferManager->EnableBuffer(geo->GetIndices());

	BufferManagerD3D9* d3d9Manager = bufferManager.As<BufferManagerD3D9>();

	BufferManagerD3D9::VertexStream vs;
	BufferManagerD3D9::IndexStream is;
	d3d9Manager->GetVertexStream(0, vs);
	d3d9Manager->GetIndexStream(is);

	const EPrimitiveType pt = geo->GetPrimitiveType();
	const u32 vertexCount = geo->GetVertexCount();
	const VertexFormat& vertexFormat = geo->GetVertexFormat();
	const EIndexFormat indexType = geo->GetIndices() ?
		geo->GetIndexType() :
		EIndexFormat::Bit16;

	if(vs.data) {
		DrawPrimitiveList(
			pt,
			firstPrimitive,
			primitiveCount,
			vs.data,
			vertexCount,
			vertexFormat,
			is.data,
			indexType,
			is3D, true);
	} else {
		DrawPrimitiveList(
			pt,
			firstPrimitive,
			primitiveCount,
			geo->GetVertices(),
			vertexCount,
			vertexFormat,
			geo->GetIndices(),
			indexType,
			is3D, false);
	}
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::SetupRendering(size_t passId)
{
	RenderSettings settings(
		m_FinalOverwrite,
		**m_Material);

	auto newRenderer = m_UseMaterial ? m_Material->GetRenderer() : nullptr;

	if(m_MaterialRenderer != newRenderer) {
		// Force update all.
		SetDirty(Dirty_MaterialRenderer);
		m_MaterialRenderer = newRenderer;
	}

	// Get current pass, with applied options and overwrites
	Pass pass = m_MaterialRenderer ? m_MaterialRenderer->GeneratePassData(passId, settings) : m_Pass;
	if(m_UseOverwrite)
		m_FinalOverwrite.Apply(pass);

	if((pass.shader != nullptr) != m_UseShader) {
		SetDirty(Dirty_Fog);
		SetDirty(Dirty_Lights);
		m_UseShader = (pass.shader != nullptr);
	}

	if(pass.lighting != m_PrevLighting) {
		SetDirty(Dirty_Lights);
		m_PrevLighting = pass.lighting;
	}

	if(pass.polygonOffset != m_PrePolyOffset) {
		SetDirty(Dirty_PolygonOffset);
		m_PrePolyOffset = pass.polygonOffset;
	}

	// Enable pass settings
	if(m_MaterialRenderer)
		m_DeviceState.SetD3DColors(*m_ParamId.ambient, settings.material, pass.lighting);
	m_DeviceState.EnablePass(pass);

	// Generate data for transforms, fog and light
	LoadTransforms(pass);
	LoadFogSettings(pass);
	LoadLightSettings(pass);

	// Send the generated data to the shader
	if(m_MaterialRenderer)
		m_MaterialRenderer->SendShaderSettings(passId, pass, settings);
	else if(m_ParamSetCallback)
		m_ParamSetCallback->SendShaderSettings(passId, pass, settings);

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

	if(m_RenderMode != mode)
		SetDirty(Dirty_RenderMode);

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
}

void RendererD3D9::LeaveRenderMode2D()
{
}

void RendererD3D9::LoadTransforms(const Pass& pass)
{
	if(m_RenderMode == ERenderMode::Mode3D) {
		if(IsDirty(Dirty_PolygonOffset) || IsDirty(Dirty_RenderMode) || IsDirty(Dirty_ViewProj)) {
			math::Matrix4 projCopy = m_TransformProj; // The userset projection matrix
			if(pass.polygonOffset) {
				const u32 zBits = m_Driver->GetConfig().zsFormat.zBits;
				const u32 values = 1 << zBits;
				const float min = 1.0f / values;
				projCopy.AddTranslation(math::Vector3F(0.0f, 0.0f, -min * pass.polygonOffset));
			}
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, projCopy);
			SetDirty(Dirty_ViewProj);
		}
	} else if(m_RenderMode == ERenderMode::Mode2D) {
		if(IsDirty(Dirty_Rendertarget) || IsDirty(Dirty_RenderMode) || IsDirty(Dirty_ViewProj)) {
			auto ssize = GetRenderTarget().GetSize();
			math::Matrix4 view = math::Matrix4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				-(float)ssize.width / 2 - 0.5f, (float)ssize.height / 2 - 0.5f, 0.0f, 1.0f);

			math::Matrix4 proj = math::Matrix4(
				2.0f / ssize.width, 0.0f, 0.0f, 0.0f,
				0.0f, 2.0f / ssize.height, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m_MatrixTable.SetMatrix(MatrixTable::MAT_VIEW, view);
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, proj);

			SetDirty(Dirty_ViewProj);
		}
	}

	if(IsDirty(Dirty_ViewProj)) {
		m_DeviceState.SetTransform(D3DTS_PROJECTION, m_MatrixTable.GetMatrix(MatrixTable::MAT_PROJ));
		m_DeviceState.SetTransform(D3DTS_VIEW, m_MatrixTable.GetMatrix(MatrixTable::MAT_VIEW));
		ClearDirty(Dirty_ViewProj);
	}
	if(IsDirty(Dirty_World)) {
		m_DeviceState.SetTransform(D3DTS_WORLD, m_MatrixTable.GetMatrix(MatrixTable::MAT_WORLD));
		ClearDirty(Dirty_World);
	}

	ClearDirty(Dirty_PolygonOffset);
}

void RendererD3D9::LoadFogSettings(const Pass& pass)
{
	bool useFog = pass.fogEnabled && m_Fog.isActive;
	if(m_RenderMode == ERenderMode::Mode2D)
		useFog = false;

	bool useFixedFog = useFog;
	if(pass.shader)
		useFixedFog = false;

	m_DeviceState.EnableFog(useFixedFog);

	if(IsDirty(Dirty_Fog)) {
		math::Vector3F fogInfo;
		fogInfo.x = useFog;
		fogInfo.y =
			m_Fog.type == EFogType::Linear ? 1.0f :
			m_Fog.type == EFogType::Exp ? 2.0f :
			m_Fog.type == EFogType::ExpSq ? 3.0f : 1.0f;
		fogInfo.z = 0;
		*m_ParamId.fogInfo = fogInfo;

		*m_ParamId.fogColor = m_Fog.color;
		*m_ParamId.fogRange = math::Vector3F(
			m_Fog.start,
			m_Fog.end,
			m_Fog.density);

		// Only use the fixed pipeline if there is no shader
		m_DeviceState.SetFog(m_Fog);
		ClearDirty(Dirty_Fog);
	}
}

void RendererD3D9::LoadLightSettings(const Pass& pass)
{
	bool useLights = (pass.lighting != ELighting::Disabled);
	if(m_RenderMode == ERenderMode::Mode2D)
		useLights = false;

	bool useFixedLights = useLights;
	if(pass.shader)
		useFixedLights = false;

	m_DeviceState.EnableLight(useFixedLights);

	if(IsDirty(Dirty_Lights)) {
		m_DeviceState.ClearLights();
		*m_ParamId.lighting = (float)pass.lighting;

		// Only use the fixed pipeline if there is no shader
		if(!pass.shader) {
			// Enable fixed pipeline lights
			for(auto it = m_Lights.First(); it != m_Lights.End(); ++it)
				m_DeviceState.AddLight(*it, pass.lighting);
		} else {
			// Generate light matrices and set as param.
			for(size_t i = 0; i < m_Lights.Size(); ++i)
				*m_ParamId.lights[i] = GenerateLightMatrix(m_Lights[i], true);
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
