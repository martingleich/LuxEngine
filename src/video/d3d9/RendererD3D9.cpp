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
#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "platform/D3D9Exception.h"

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
	core::AttributeListBuilder alb;
	m_ParamIds.lighting = alb.AddAttribute("lighting", (float)video::ELightingFlag::Enabled);
	m_ParamIds.fogEnabled = alb.AddAttribute("fogEnabled", 1.0f);

	m_ParamIds.ambient = alb.AddAttribute("ambient", video::ColorF(0, 0, 0));
	m_ParamIds.time = alb.AddAttribute("time", 0.0f);

	for(auto a : m_MatrixTable.Attributes())
		alb.AddAttribute(a);

	m_Params = m_BaseParams = alb.BuildAndReset();

	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_ScissorRect.Set(0, 0, m_BackbufferTarget.GetSize().width, m_BackbufferTarget.GetSize().height);
	m_CurrentRendertargets.PushBack(m_BackbufferTarget);
}

RendererD3D9::~RendererD3D9()
{
	ReleaseUnmanaged();

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
			throw core::InvalidOperationException("Scene was already started");
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
		D3DCOLOR d3dClear = color.ToDWORD();
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
			throw core::InvalidOperationException("Scene was already started");
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
		throw core::GenericInvalidArgumentException("count", "There must be at least one rendertarget.");
	if(count > m_Driver->GetDeviceCapability(EDriverCaps::MaxSimultaneousRT))
		throw core::GenericInvalidArgumentException("count", "Too many rendertargets.");

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
					throw core::GenericInvalidArgumentException("target", "All rendertargets must have the same size");
				if(!t.GetTexture()->IsRendertarget())
					throw core::GenericInvalidArgumentException("target", "Must be a rendertarget texture");
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
		throw core::GenericRuntimeException("Can't find matching depth buffer for rendertarget.");

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

void RendererD3D9::SetScissorRect(const math::RectI& rect, ScissorRectToken* token)
{
	auto bufferSize = GetRenderTarget().GetSize();
	math::RectI fittedRect = rect;
	fittedRect.FitInto(math::RectI(0, 0, bufferSize.width, bufferSize.height));
	if(!fittedRect.IsValid())
		throw core::GenericInvalidArgumentException("rect", "Rectangle is invalid");

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

void RendererD3D9::SetTransform(ETransform transform, const math::Matrix4& matrix)
{
	switch(transform) {
	case ETransform::World:
		m_MatrixTable.SetMatrix(MatrixTable::MAT_WORLD, matrix);
		break;
	case ETransform::View:
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

const math::Matrix4& RendererD3D9::GetTransform(ETransform transform) const
{
	switch(transform) {
	case ETransform::World: return m_MatrixTable.GetMatrix(MatrixTable::MAT_WORLD);
	case ETransform::View: return m_MatrixTable.GetMatrix(MatrixTable::MAT_VIEW);
	case ETransform::Projection: return m_MatrixTable.GetMatrix(MatrixTable::MAT_PROJ);
	default: throw core::GenericInvalidArgumentException("transform", "Unknown transform");
	}
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::Draw(const RenderRequest& rq)
{
	if(rq.primitiveCount == 0)
		return;

	u32 vertexCount;
	u32 vertexOffset;
	u32 indexOffset;
	const VertexFormat* vformat;
	EIndexFormat iformat;
	// Enable buffers and calculate offsets.
	if(!rq.userPointer) {
		// Rendering from hardwarebuffer
		auto bufferManager = m_Driver->GetBufferManager();

		bufferManager->EnableBuffer(rq.bufferData.vb);

		BufferManagerD3D9* d3d9Manager = bufferManager.As<BufferManagerD3D9>();
		vformat = &rq.bufferData.vb->GetFormat();
		vertexCount = rq.bufferData.vb->GetSize();

		BufferManagerD3D9::VertexStream vs;
		if(d3d9Manager->GetVertexStream(vs))
			vertexOffset = vs.offset;
		else
			vertexOffset = 0;
		if(rq.indexed) {
			iformat = rq.bufferData.ib->GetFormat();
			BufferManagerD3D9::IndexStream is;
			bufferManager->EnableBuffer(rq.bufferData.ib);
			if(d3d9Manager->GetIndexStream(is))
				indexOffset = is.offset;
			else
				indexOffset = 0;
		} else {
			iformat = EIndexFormat::Bit16;
			indexOffset = 0;
		}
	} else {
		vertexOffset = 0;
		indexOffset = 0;
		vformat = rq.userData.vertexFormat;
		iformat = rq.userData.indexFormat;
		vertexCount = rq.userData.vertexCount;
	}

	// Calculate offsets for first primitive.
	if(rq.indexed)
		indexOffset += video::GetPointCount(rq.primitiveType, rq.firstPrimitive);
	else
		vertexOffset += video::GetPointCount(rq.primitiveType, rq.firstPrimitive);

	D3DPRIMITIVETYPE d3dPrimitiveType = GetD3DPrimitiveType(rq.primitiveType);

	SetVertexFormat(*vformat);

	EFaceSide cullMode = m_CurPassCullMode;
	if(rq.frontFace == EFaceWinding::CCW)
		(void)0;
	else if(rq.frontFace == EFaceWinding::CW)
		cullMode = FlipFaceSide(m_CurPassCullMode);
	else if(rq.frontFace == EFaceWinding::ANY)
		cullMode = EFaceSide::None;
	m_DeviceState.SetRenderState(D3DRS_CULLMODE, GetD3DCullMode(cullMode));

	HRESULT hr = E_FAIL;
	if(rq.userPointer) {
		DWORD stride = (DWORD)vformat->GetStride();
		if(rq.indexed) {
			DWORD indexStride = iformat == EIndexFormat::Bit16 ? 2 : 4;
			D3DFORMAT d3dIndexFormat = GetD3DIndexFormat(iformat);
			auto indexData = (u8*)rq.userData.indexData + indexOffset * indexStride;
			hr = m_Device->DrawIndexedPrimitiveUP(d3dPrimitiveType, 0, vertexCount, rq.primitiveCount, indexData, d3dIndexFormat, rq.userData.vertexData, stride);
		} else {
			auto vertexData = (u8*)rq.userData.vertexData + vertexOffset * stride;
			hr = m_Device->DrawPrimitiveUP(d3dPrimitiveType, rq.primitiveCount, vertexData, stride);
		}
	} else {
		if(rq.indexed)
			hr = m_Device->DrawIndexedPrimitive(d3dPrimitiveType, 0, 0, vertexCount, indexOffset, rq.primitiveCount);
		else
			hr = m_Device->DrawPrimitive(d3dPrimitiveType, vertexOffset, rq.primitiveCount);
	}

	if(FAILED(hr)) {
		auto err = core::D3D9Exception::MakeErrorString(hr);
		log::Error("Error while drawing: {}.", err.AsView());
		return;
	}

	if(m_RenderStatistics)
		m_RenderStatistics->AddPrimitives(rq.primitiveCount);
}

void RendererD3D9::SendPassSettingsEx(
	ERenderMode newRenderMode,
	const Pass& _pass,
	bool useOverwrite,
	ShaderParamSetCallback*
	paramSetCallback,
	void* userParam)
{
	auto pass = _pass;

	lxAssert(pass.shader != nullptr);

	// Update the pipelineSettings to fit with the configuration.
	if(useOverwrite && !m_PipelineOverwrites.IsEmpty())
		m_FinalOverwrite.Apply(pass);
	bool isDirtyRendermode = (newRenderMode != m_RenderMode);
	m_RenderMode = newRenderMode;
	if(newRenderMode == ERenderMode::Mode2D) {
		pass.lighting = ELightingFlag::Disabled;
		pass.fogEnabled = false;
	}

	m_CurPassCullMode = pass.culling;

	bool changedFogEnable = (pass.fogEnabled != m_PrevFog);
	bool changedLighting = (pass.lighting != m_PrevLighting);
	bool changedPolygonOffset = (pass.polygonOffset != m_PrevPolyOffset);

	// Enable shader
	if(pass.shader != m_CurrentShader) {
		pass.shader->Enable();
		m_CurrentShader = pass.shader;
	}
	
	// Enable pass
	m_DeviceState.EnableAlpha(pass.alpha);
	m_DeviceState.SetStencilMode(pass.stencil);
	m_DeviceState.SetRenderState(D3DRS_COLORWRITEENABLE, pass.colorMask);

	m_DeviceState.SetRenderState(D3DRS_ZFUNC, GetD3DComparisonFunc(pass.zBufferFunc));
	m_DeviceState.SetRenderState(D3DRS_ZWRITEENABLE, pass.zWriteEnabled ? TRUE : FALSE);
	m_DeviceState.SetRenderState(D3DRS_FILLMODE, GetD3DFillMode(pass.drawMode));
	m_DeviceState.SetRenderState(D3DRS_SHADEMODE, pass.gouraudShading ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);

	m_DeviceState.SetRenderState(D3DRS_NORMALIZENORMALS, m_NormalizeNormals ? TRUE : FALSE);

	// Update projection matrices to include polygon offset, or change for 2D mode
	if(m_RenderMode == ERenderMode::Mode3D) {
		if(isDirtyRendermode || changedPolygonOffset || IsDirty(Dirty_ViewProj)) {
			math::Matrix4 projCopy = m_TransformProj; // The userset projection matrix
			if(pass.polygonOffset) {
				const u8 zBits = m_Driver->GetConfig().zsFormat.zBits;
				const u32 values = 1 << zBits;
				const float min = 1.0f / values;
				projCopy.AddTranslation(math::Vector3F(0, 0, -min * pass.polygonOffset));
			}
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, projCopy);
		}
	} else if(m_RenderMode == ERenderMode::Mode2D) {
		if(isDirtyRendermode || IsDirty(Dirty_Rendertarget) || IsDirty(Dirty_ViewProj)) {
			auto ssize = GetRenderTarget().GetSize();
			math::Matrix4 view = math::Matrix4(
				1,  0, 0, 0,
				0, -1, 0, 0,
				0,  0, 1, 0,
				-(float)ssize.width / 2 - 0.5f, (float)ssize.height / 2 + 0.5f, 0, 1);

			math::Matrix4 proj = math::Matrix4(
				2.0f / (float)ssize.width, 0.0f, 0.0f, 0.0f,
				0.0f, 2.0f / (float)ssize.height, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m_MatrixTable.SetMatrix(MatrixTable::MAT_VIEW, view);
			m_MatrixTable.SetMatrix(MatrixTable::MAT_PROJ, proj);
		}
	}

	// Generate data for fog and light
	if(changedFogEnable)
		m_ParamIds.fogEnabled->SetValue<float>(pass.fogEnabled ? 1.0f : 0.0f); 

	if(changedLighting)
		m_ParamIds.lighting->SetValue<float>(float(pass.lighting));

	// Send the generated data to the shader
	// Only if scene or shader changed.
	pass.shader->LoadSceneParams(GetParams(), pass);

	// Let the user fill in parameters.
	// TODO: Allow user to do this per hand.
	if(paramSetCallback)
		paramSetCallback->SendShaderSettings(pass, userParam);

	// Start rendering with this shader.
	// TODO: Move into Draw function.
	pass.shader->Render();

	// TODO: Remove as many Dirtys from here into the shader as possible.
	ClearDirty(Dirty_ViewProj);
	ClearDirty(Dirty_Rendertarget);
	m_PrevFog = pass.fogEnabled;
	m_PrevLighting = pass.lighting;
	m_PrevPolyOffset = pass.polygonOffset;
}

///////////////////////////////////////////////////////////////////////////

void RendererD3D9::ReleaseUnmanaged()
{
	m_CurrentRendertargets.Clear();
	m_BackbufferTarget = RendertargetD3D9(nullptr);
	m_CurrentShader = nullptr;
}

void RendererD3D9::Reset()
{
	m_DirtyFlags = 0xFFFFFFFF;

	m_BackbufferTarget = m_Driver->GetBackbufferTarget();
	m_ScissorRect.Set(0, 0, m_BackbufferTarget.GetSize().width, m_BackbufferTarget.GetSize().height);
	m_CurrentRendertargets.PushBack(m_BackbufferTarget);
	m_CurrentShader = nullptr;
}

///////////////////////////////////////////////////////////////////////////

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
