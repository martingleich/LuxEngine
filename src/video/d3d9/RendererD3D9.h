#ifndef INCLUDED_RENDERER_D3D9_H
#define INCLUDED_RENDERER_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/RendererNull.h"

#include "StrippedD3D9.h"

#include "video/d3d9/D3DHelper.h"
#include "video/d3d9/UnknownRefCounted.h"
#include "video/d3d9/RenderTargetD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"

namespace lux
{
namespace video
{
class VideoDriverD3D9;

class RendererD3D9 : public RendererNull
{
public:
	RendererD3D9(VideoDriverD3D9* driver);

	void CleanUp();

	void BeginScene(
		bool clearColor, bool clearZBuffer, bool clearStencil,
		video::Color color = video::Color::Black, float z = 1.0f, u32 stencil=0);

	void EndScene();
	bool Present();

	void SetRenderTarget(const RenderTarget& target);
	const RenderTarget& GetRenderTarget();

	void SetScissorRect(const math::RectU& rect, ScissorRectToken* token=nullptr);
	const math::RectU& GetScissorRect() const;

	///////////////////////////////////////////////////////////////////////////

	size_t GetMaxLightCount() const;

	///////////////////////////////////////////////////////////////////////////

	void DrawPrimitiveList(
		EPrimitiveType primitiveType, u32 firstPrimitive, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		bool is3D, bool user);

	void DrawIndexedPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		const void* indexData, EIndexFormat indexType,
		bool is3D)
	{
		LX_CHECK_NULL_ARG(vertexData);
		LX_CHECK_NULL_ARG(indexData);

		return DrawPrimitiveList(primitiveType, 0, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			indexData, indexType,
			is3D, true);
	}

	void DrawPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
		const void* vertexData, u32 vertexCount, const VertexFormat& vertexFormat,
		bool is3D)
	{
		LX_CHECK_NULL_ARG(vertexData);

		return DrawPrimitiveList(primitiveType, 0, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			nullptr, EIndexFormat::Bit16,
			is3D, true);
	}

	void DrawGeometry(const Geometry* geo, bool is3D = true)
	{
		RendererD3D9::DrawGeometry(geo, 0xFFFFFFFF, is3D);
	}

	void DrawGeometry(const Geometry* geo, u32 firstPrimitive, u32 primitiveCount, bool is3D = true);

	///////////////////////////////////////////////////////////////////////////
private:
	void SetupRendering(size_t passId);

	void SwitchRenderMode(ERenderMode mode);
	void EnterRenderMode3D();
	void LeaveRenderMode3D();
	void EnterRenderMode2D();
	void LeaveRenderMode2D();

	void LoadTransforms(const Pass& pass);
	void LoadFogSettings(const Pass& pass);
	void LoadLightSettings(const Pass& pass);

	void SetVertexFormat(const VertexFormat& format);

private:
	IDirect3DDevice9* m_Device;
	DeviceStateD3D9 m_DeviceState;

	RendertargetD3D9 m_CurrentRendertarget;
	RendertargetD3D9 m_BackbufferTarget;

	math::RectU m_ScissorRect;

	VideoDriverD3D9* m_Driver;

	// The current state of rendersettings, for optimization purposes
	MaterialRenderer* m_MaterialRenderer;
	VertexFormat m_VertexFormat;
	bool m_UseShader=false;
	float m_PrePolyOffset=0.0f;
	ELighting m_PrevLighting = ELighting::Disabled;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_RENDERER_D3D9_H
