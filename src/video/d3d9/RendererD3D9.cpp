#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "RendererD3D9.h"

#include "video/TextureLayer.h"
#include "video/BaseTexture.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexTypes.h"
#include "video/Shader.h"
#include "core/Logger.h"

#include "video/d3d9/VideoDriverD3D9.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"
#include "platform/D3D9Exception.h"
#include "video/FixedFunctionShader.h"

namespace lux
{
namespace video
{

RendererD3D9::RendererD3D9(VideoDriverD3D9* driver, DeviceStateD3D9& deviceState) :
	RendererNull(driver),
	m_Device((IDirect3DDevice9*)driver->GetLowLevelDevice()),
	m_DeviceState(deviceState),
	m_Driver(driver)
{
	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_ScissorRect.Set(0, 0, m_BackbufferTarget.GetSize().width, m_BackbufferTarget.GetSize().height);
	m_CurrentRendertargets.PushBack(m_BackbufferTarget);
	m_ActiveFixedLights = 0;
}

RendererD3D9::~RendererD3D9()
{
	// Free all bound objects
	m_Device->SetVertexDeclaration(nullptr);
	m_Device->SetVertexShader(nullptr);
	m_Device->SetPixelShader(nullptr);

	// Index and Vertexbuffer are unbound in Hardwarebuffermanager destructor.
}

void RendererD3D9::BeginScene()
{
	HRESULT hr = m_Device->BeginScene();
	if(FAILED(hr)) {
		if(hr == D3DERR_INVALIDCALL)
			throw core::Exception("Scene was already started");
		else
			throw core::D3D9Exception(hr);
	}
}

void RendererD3D9::Clear(
	bool clearColor, bool clearZBuffer, bool clearStencil,
	video::Color color, float z, u32 stencil)
{
	u32 flags = 0;
	if(clearColor)
		flags = D3DCLEAR_TARGET;
	if(clearZBuffer)
		flags |= D3DCLEAR_ZBUFFER;
	if(clearStencil && m_Driver->GetConfig().zsFormat.sBits != 0)
		flags |= D3DCLEAR_STENCIL;

	HRESULT hr = S_OK;

	if(flags) {
		const D3DCOLOR d3dClear = color.ToDWORD();
		if(FAILED(hr = m_Device->Clear(
			0, nullptr,
			flags,
			d3dClear, z, stencil))) {
			throw core::D3D9Exception(hr);
		}
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
}

bool RendererD3D9::Present()
{
	HRESULT hr = m_Device->Present(NULL, NULL, NULL, NULL);

	return SUCCEEDED(hr);
}

void RendererD3D9::SetRenderTarget(const RenderTarget& target)
{
	if(m_CurrentRendertargets.Size() == 1 && target.GetTexture() == m_CurrentRendertargets[0].GetTexture())
		return;

	SetRenderTarget(&target, 1, false);
}

void RendererD3D9::SetRenderTarget(const core::Array<RenderTarget>& targets)
{
	SetRenderTarget(targets.Data(), targets.Size(), false);
}

void RendererD3D9::SetRenderTarget(const RenderTarget* targets, int count, bool restore)
{
	if(count == 0)
		throw core::InvalidArgumentException("count", "There must be at least one rendertarget.");
	if(count > m_Driver->GetDeviceCapability(EDriverCaps::MaxSimultaneousRT))
		throw core::InvalidArgumentException("count", "To many rendertargets.");

	// Check if textures are valid
	math::Dimension2I dim;
	if(!targets[0].IsBackbuffer())
		dim = targets[0].GetSize();
	else
		dim = m_BackbufferTarget.GetSize();

	{
		bool isSet = true;
		int i = 0;
		for(auto& t : core::MakeRange(targets, targets + count)) {
			if(!t.IsBackbuffer()) {
				if(t.GetSize() != dim)
					throw core::InvalidArgumentException("target", "All rendertargets must have the same size");
				if(!t.GetTexture()->IsRendertarget())
					throw core::InvalidArgumentException("target", "Must be a rendertarget texture");
			}
			if(t.GetTexture() != m_CurrentRendertargets[i].GetTexture()) {
				isSet = false;
				continue;
			}
			++i;
		}

		// Early out if everything is already set.
		if(isSet)
			return;
	}

	RendertargetD3D9 newRendertarget;
	if(targets[0].IsBackbuffer())
		newRendertarget = m_BackbufferTarget;
	else
		newRendertarget = targets[0];

	// Generate depth buffer via video driver
	auto depthStencil = m_Driver->GetD3D9MatchingDepthBuffer(newRendertarget.GetSurface());
	if(depthStencil == nullptr)
		throw core::Exception("Can't find matching depth buffer for rendertarget.");

	HRESULT hr = S_OK;
	for(int i = 0; i < count && !FAILED(hr); ++i) {
		if(targets[i].IsBackbuffer())
			newRendertarget = m_BackbufferTarget;
		else
			newRendertarget = targets[i];

		hr = m_Device->SetRenderTarget(i, newRendertarget.GetSurface());
	}

	if(!FAILED(hr)) {
		for(int i = count; i < m_CurrentRendertargets.Size() && !FAILED(hr); ++i) {
			hr = m_Device->SetRenderTarget(i, nullptr);
		}
	}

	if(!FAILED(hr)) {
		hr = m_Device->SetDepthStencilSurface(depthStencil);
	}

	if(FAILED(hr)) {
		if(!restore) {
			// Restore backbuffer as rendertarget.
			SetRenderTarget(nullptr, 1, true);
		}
		throw core::D3D9Exception(hr);
	}

	m_CurrentRendertargets.Clear();
	for(int i = 0; i < count; ++i) {
		if(targets[i].IsBackbuffer())
			newRendertarget = m_BackbufferTarget;
		else
			newRendertarget = targets[i];
		m_CurrentRendertargets.PushBack(newRendertarget);
	}

	// Reset scissor rectangle
	SetScissorRect(math::RectI(0, 0, dim.width, dim.height));
}

const RenderTarget& RendererD3D9::GetRenderTarget()
{
	return m_CurrentRendertargets[0];
}

const math::Dimension2I& RendererD3D9::GetRenderTargetSize()
{
	return m_CurrentRendertargets[0].GetSize();
}

void RendererD3D9::SetScissorRect(const math::RectI& rect, ScissorRectToken* token)
{
	auto bufferSize = GetRenderTarget().GetSize();
	math::RectI fittedRect = rect;
	fittedRect.FitInto(math::RectI(0, 0, bufferSize.width, bufferSize.height));
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

const math::RectI& RendererD3D9::GetScissorRect() const
{
	return m_ScissorRect;
}

///////////////////////////////////////////////////////////////////////////

int RendererD3D9::GetMaxLightCount() const
{
	return m_Driver->GetDeviceCapability(EDriverCaps::MaxLights);
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::DrawPrimitiveList(
	EPrimitiveType primitiveType, u32 firstPrimitive, u32 primitiveCount,
	const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
	const void* indexData, EIndexFormat indexType,
	bool is3D, EFaceWinding frontFace, bool user)
{
	if(primitiveCount == 0)
		return;

	u32 stride = vertexFormat.GetStride(0);
	u32 indexStride = indexType == EIndexFormat::Bit16 ? 2 : 4;

	D3DFORMAT d3dIndexFormat = GetD3DIndexFormat(indexType);
	D3DPRIMITIVETYPE d3dPrimitiveType = GetD3DPrimitiveType(primitiveType);

	u32 vertexOffset = 0;
	u32 indexOffset = 0;
	if(!user) {
		if(indexData) {
			// Indexed from stream
			auto d3d9Manager = m_Driver->GetBufferManager().StaticCastStrong<BufferManagerD3D9>();

			BufferManagerD3D9::VertexStream vs;
			BufferManagerD3D9::IndexStream is;

			if(d3d9Manager->GetVertexStream(0, vs))
				vertexOffset = vs.offset;

			if(d3d9Manager->GetIndexStream(is))
				indexOffset = is.offset;
		} else {
			// Not indexed from stream
			auto d3d9Manager = m_Driver->GetBufferManager().StaticCastStrong<BufferManagerD3D9>();

			BufferManagerD3D9::VertexStream vs;
			if(d3d9Manager->GetVertexStream(0, vs))
				vertexOffset = vs.offset;
		}
	}

	if(indexData)
		indexOffset += video::GetPointCount(primitiveType, firstPrimitive);
	else
		vertexOffset += video::GetPointCount(primitiveType, firstPrimitive);

	SetVertexFormat(vertexFormat);
	SwitchRenderMode(is3D ? ERenderMode::Mode3D : ERenderMode::Mode2D);

	u32 passCount = m_Material ? m_Material->GetPassCount() : 1;
	for(u32 pass = 0; pass < passCount; ++pass) {
		if(pass > 0) {
			// For the first pass it was set in SetMaterial
			SetDirty(Dirty_Material);
		}

		SetupRendering(frontFace, pass);
		HRESULT hr = E_FAIL;
		if(!user) {
			if(indexData)
				hr = m_Device->DrawIndexedPrimitive(d3dPrimitiveType, 0, 0, vertexCount, indexOffset, primitiveCount);
			else
				hr = m_Device->DrawPrimitive(d3dPrimitiveType, vertexOffset, primitiveCount);
		} else {
			if(indexData) { // Indexed from memory
				hr = m_Device->DrawIndexedPrimitiveUP(d3dPrimitiveType, 0, vertexCount, primitiveCount,
					(u8*)indexData + indexOffset*indexStride, d3dIndexFormat, vertexData, stride);
			} else { // Not indexed from memory
				hr = m_Device->DrawPrimitiveUP(d3dPrimitiveType, primitiveCount, (u8*)vertexData + vertexOffset*stride, stride);
			}
		}
		if(FAILED(hr)) {
			log::Error("Error while drawing.");
			return;
		}
	}

	if(passCount > 1) {
		// The next rendercall will start with another pass.
		SetDirty(Dirty_Material);
	}

	if(m_RenderStatistics)
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
	const EIndexFormat indexFormat = geo->GetIndices() ?
		geo->GetIndexFormat() :
		EIndexFormat::Bit16;

	if(vs.data) {
		DrawPrimitiveList(
			pt,
			firstPrimitive, primitiveCount,
			vs.data, vertexCount, vertexFormat,
			is.data, indexFormat,
			is3D, geo->GetFrontFaceWinding(),
			true);
	} else {
		DrawPrimitiveList(
			pt,
			firstPrimitive, primitiveCount,
			geo->GetVertices(), vertexCount, vertexFormat,
			geo->GetIndices(), indexFormat,
			is3D, geo->GetFrontFaceWinding(),
			false);
	}
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::ReleaseUnmanaged()
{
	m_CurrentRendertargets.Clear();
	m_BackbufferTarget = RendertargetD3D9(nullptr);
}

void RendererD3D9::Reset()
{
	m_DirtyFlags = 0xFFFFFFFF;

	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_ScissorRect.Set(0, 0, m_BackbufferTarget.GetSize().width, m_BackbufferTarget.GetSize().height);
	m_CurrentRendertargets.PushBack(m_BackbufferTarget);
	m_ActiveFixedLights = 0;
}

///////////////////////////////////////////////////////////////////////////
void RendererD3D9::SetupRendering(EFaceWinding frontFace, u32 passId)
{
	bool dirtyPass = IsDirty(Dirty_Pass | Dirty_Material);
	auto pass = m_Material ? m_Material->GetPass(passId) : m_Pass;

	lxAssert(pass.shader != nullptr);

	if(m_UseOverwrite && !m_PipelineOverwrites.IsEmpty()) {
		m_FinalOverwrite.Apply(pass);
		dirtyPass = true;
	}
	if(m_RenderMode == ERenderMode::Mode2D) {
		pass.lighting = ELightingFlag::Disabled;
		pass.fogEnabled = false;
		dirtyPass = true;
	}
	if(frontFace != EFaceWinding::CCW) {
		pass.culling = FlipFaceSide(pass.culling);
		dirtyPass = true;
	}
	if(pass.fogEnabled != m_IsFogActive && pass.fogEnabled) {
		pass.fogEnabled = m_IsFogActive && pass.fogEnabled;
		dirtyPass = true;
	}

	bool changedFogEnable = false;
	bool changedLighting = false;
	bool changedShader = false;
	bool useFixedPipeline = (pass.shader.As<FixedFunctionShader>() != nullptr);
	if(dirtyPass) {
		if(pass.fogEnabled != m_PrevFog) {
			changedFogEnable = true;
		}
		if(pass.lighting != m_PrevLighting) {
			changedLighting = true;
		}

		if(pass.polygonOffset != m_PrevPolyOffset) {
			SetDirty(Dirty_PolygonOffset);
		}

		// Enable pass settings
		// First enable shader to mimize stage changes.
		auto oldShader = m_DeviceState.GetShader();
		auto oldUseFixedPipeline = m_DeviceState.IsFixedShader();
		if(pass.shader != oldShader) {
			m_DeviceState.EnableShader(pass.shader);

			// If we switches from a fixed shader to another fixed shader, it's no real switch.
			changedShader = true;
			if(oldUseFixedPipeline && useFixedPipeline && oldShader)
				changedShader = false;
			else
				changedShader = true;
		}
		m_DeviceState.EnablePass(pass, *m_ParamId.ambient);
	}

	m_DeviceState.SetRenderState(D3DRS_NORMALIZENORMALS, m_NormalizeNormals ? TRUE : FALSE);

	// Update projection matrices to include polygon offset, or change for 2D mode
	UpdateTransforms(pass.polygonOffset);
	ClearDirty(Dirty_PolygonOffset);

	// Load fixed function transforms
	if(useFixedPipeline) {
		// Only if matrix changed or shader changed
		if(IsDirty(Dirty_ViewProj) || changedShader) {
			m_DeviceState.SetTransform(D3DTS_PROJECTION, m_MatrixTable.GetMatrix(MatrixTable::MAT_PROJ));
			m_DeviceState.SetTransform(D3DTS_VIEW, m_MatrixTable.GetMatrix(MatrixTable::MAT_VIEW));
		}
		if(IsDirty(Dirty_World) || changedShader) {
			m_DeviceState.SetTransform(D3DTS_WORLD, m_MatrixTable.GetMatrix(MatrixTable::MAT_WORLD));
		}
	}
	ClearDirty(Dirty_World);
	ClearDirty(Dirty_ViewProj);

	// Generate data for fog and light
	LoadFogSettings(pass.fogEnabled, useFixedPipeline, changedShader, changedFogEnable);
	LoadLightSettings(pass.lighting, useFixedPipeline, changedShader, changedLighting);

	// Send the generated data to the shader
	// Only if scene or shader changed.
	pass.shader->LoadSceneParams(pass);

	if(m_ParamSetCallback && pass.shader)
		m_ParamSetCallback->SendShaderSettings(passId, pass, m_UserParam);

	// Start rendering with this shader
	pass.shader->Render();

	ClearDirty(Dirty_Rendertarget);
	ClearDirty(Dirty_Material);
	ClearDirty(Dirty_RenderMode);
	ClearDirty(Dirty_Overwrites);
	m_PrevFog = pass.fogEnabled;
	m_PrevLighting = pass.lighting;
	m_PrevPolyOffset = pass.polygonOffset;
}

void RendererD3D9::SwitchRenderMode(ERenderMode mode)
{
	if(m_RenderMode == mode)
		return;

	SetDirty(Dirty_RenderMode);

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
}

void RendererD3D9::LeaveRenderMode2D()
{
}

void RendererD3D9::UpdateTransforms(float polygonOffset)
{
	if(m_RenderMode == ERenderMode::Mode3D) {
		if(IsDirty(Dirty_PolygonOffset) || IsDirty(Dirty_RenderMode) || IsDirty(Dirty_ViewProj)) {
			math::Matrix4 projCopy = m_TransformProj; // The userset projection matrix
			if(polygonOffset) {
				const u32 zBits = m_Driver->GetConfig().zsFormat.zBits;
				const u32 values = 1 << zBits;
				const float min = 1.0f / values;
				projCopy.AddTranslation(math::Vector3F(0.0f, 0.0f, -min * polygonOffset));
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
				-(float)ssize.width / 2 - 0.5f, (float)ssize.height / 2 + 0.5f, 0.0f, 1.0f);

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
}

void RendererD3D9::LoadFogSettings(
	bool isFogActive,
	bool fixedFunction,
	bool changedShader,
	bool changedFogState)
{
	bool useFixedFog = fixedFunction && isFogActive;

	// Change if not using fixed function, and the fog or shader changed
	if(!fixedFunction && (IsDirty(Dirty_Fog) || changedShader || changedFogState)) {
		// Set shader fog state.
		video::ColorF fog1, fog2;
		fog1.r = m_Fog.color.r;
		fog1.g = m_Fog.color.g;
		fog1.b = m_Fog.color.b;
		fog1.a = isFogActive ? 1.0f : 0.0f;
		fog2.r =
			m_Fog.type == EFogType::Linear ? 1.0f :
			m_Fog.type == EFogType::Exp ? 2.0f :
			m_Fog.type == EFogType::ExpSq ? 3.0f : 1.0f;
		fog2.g = m_Fog.start;
		fog2.b = m_Fog.end;
		fog2.a = m_Fog.density;

		*m_ParamId.fog1 = fog1;
		*m_ParamId.fog2 = fog2;
	}
	// Change if using fixed function, and the fog or shader changed
	if(fixedFunction && (IsDirty(Dirty_Fog) || changedShader || changedFogState)) {
		m_DeviceState.SetRenderState(D3DRS_FOGENABLE, useFixedFog ? TRUE : FALSE);
		m_DeviceState.SetFog(m_Fog);
	}

	ClearDirty(Dirty_Fog);
}

void RendererD3D9::LoadLightSettings(
	ELightingFlag lighting,
	bool fixedFunction,
	bool changedShader,
	bool changedLighting)
{
	bool useLights = (lighting != ELightingFlag::Disabled);

	if(!fixedFunction && (IsDirty(Dirty_Lights) || changedShader || changedLighting)) {
		*m_ParamId.lighting = (float)lighting;
	}
	if(fixedFunction && (IsDirty(Dirty_Lights) || changedShader || changedLighting)) {
		m_DeviceState.EnableLight(useLights);
	}

	if(IsDirty(Dirty_Lights) || changedLighting) {
		if(fixedFunction) {
			if(useLights) {
				u32 i = 0;
				for(auto& l : m_Lights)
					m_DeviceState.SetLight(i++, l, lighting);
				u32 newLightCount = i;
				for(; i < m_ActiveFixedLights; ++i)
					m_DeviceState.DisableLight(i);
				m_ActiveFixedLights = newLightCount;
			}
		} else {
			if(IsDirty(Dirty_Lights)) {
				// Generate light matrices and set as param.
				for(int i = 0; i < m_Lights.Size(); ++i)
					*m_ParamId.lights[i] = GenerateLightMatrix(m_Lights[i], true);
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
