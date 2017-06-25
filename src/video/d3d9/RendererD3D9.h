#ifndef INCLUDED_RENDERER_D3D9_H
#define INCLUDED_RENDERER_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/RendererNull.h"
#include "video/PipelineSettings.h"

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
		bool clearColor, bool clearZBuffer,
		video::Color color = video::Color::Black, float z = 1.0f);

	void EndScene();
	bool Present();

	void SetRenderTarget(const RenderTarget& target);

	const RenderTarget& GetRenderTarget();

	///////////////////////////////////////////////////////////////////////////

	size_t GetMaxLightCount() const;

	///////////////////////////////////////////////////////////////////////////

	void DrawPrimitiveList(
		EPrimitiveType primitiveType, u32 primitiveCount,
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

		return DrawPrimitiveList(primitiveType, primitiveCount,
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

		return DrawPrimitiveList(primitiveType, primitiveCount,
			vertexData, vertexCount, vertexFormat,
			nullptr, EIndexFormat::Bit16,
			is3D, true);
	}

	void DrawGeometry(const Geometry* geo, bool is3D = true)
	{
		RendererD3D9::DrawGeometry(geo, 0xFFFFFFFF, is3D);
	}

	void DrawGeometry(const Geometry* geo, u32 primitiveCount, bool is3D = true);

	///////////////////////////////////////////////////////////////////////////
private:
	void SetupRendering(ERenderMode mode);
	void SwitchRenderMode(ERenderMode mode);
	void EnterRenderMode3D();
	void LeaveRenderMode3D();
	void EnterRenderMode2D();
	void LeaveRenderMode2D();
	void LoadSettings();
	void LoadTransforms(const RenderSettings& settings);
	void LoadFogSettings(const RenderSettings& settings);
	void LoadLightSettings(const RenderSettings& settings);
	void SetVertexFormat(const VertexFormat& format);

private:
	IDirect3DDevice9* m_Device;
	DeviceStateD3D9 m_State;

	RendertargetD3D9 m_CurrentRendertarget;
	RendertargetD3D9 m_BackbufferTarget;

	VideoDriverD3D9* m_Driver;

	// The current state of rendersettings, for optimization purposes
	PipelineSettings m_Pipeline;
	MaterialRenderer* m_MaterialRenderer;
	VertexFormat m_VertexFormat;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_RENDERER_D3D9_H
