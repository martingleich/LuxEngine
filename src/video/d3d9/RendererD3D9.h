#ifndef INCLUDED_LUX_RENDERER_D3D9_H
#define INCLUDED_LUX_RENDERER_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/RendererNull.h"

#include "platform/StrippedD3D9.h"

#include "video/d3d9/D3DHelper.h"
#include "platform/UnknownRefCounted.h"
#include "video/d3d9/RenderTargetD3D9.h"

namespace lux
{
namespace video
{
class VideoDriverD3D9;
class DeviceStateD3D9;

class RendererD3D9 : public RendererNull
{
public:
	RendererD3D9(VideoDriverD3D9* driver, DeviceStateD3D9& d3d9);
	~RendererD3D9();

	void BeginScene();
	void Clear(
		bool clearColor, bool clearZBuffer, bool clearStencil,
		video::Color color = video::Color::Black,
		float z = 1.0f,
		u32 stencil = 0);
	void EndScene();
	bool Present();

	void SetRenderTarget(const RenderTarget& target);
	void SetRenderTarget(const core::Array<RenderTarget>& targets);
	void SetRenderTarget(const RenderTarget* targets, int count, bool restore);
	const RenderTarget& GetRenderTarget();
	const math::Dimension2I& GetRenderTargetSize();

	void SetScissorRect(const math::RectI& rect, ScissorRectToken* token = nullptr);
	const math::RectI& GetScissorRect() const;

	///////////////////////////////////////////////////////////////////////////

	void Draw(const RenderRequest& rq);

	///////////////////////////////////////////////////////////////////////////
	void ReleaseUnmanaged();
	void Reset();

private:
	void SetupRendering(EFaceWinding frontFace);

	void SwitchRenderMode(ERenderMode mode);

	void UpdateTransforms(float polygonOffset);

	void SetVertexFormat(const VertexFormat& format);

private:
	UnknownRefCounted<IDirect3DDevice9> m_Device;
	DeviceStateD3D9& m_DeviceState;

	core::Array<RendertargetD3D9> m_CurrentRendertargets;
	RendertargetD3D9 m_BackbufferTarget;

	math::RectI m_ScissorRect;

	VideoDriverD3D9* m_Driver;

	VertexFormat m_VertexFormat;

	float m_PrevPolyOffset = 0.0f;
	ELightingFlag m_PrevLighting = ELightingFlag::Disabled;
	bool m_PrevFog = false;

	u32 m_ActiveFixedLights;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_LUX_RENDERER_D3D9_H
