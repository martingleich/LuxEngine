#ifndef INCLUDED_VIDEODRIVER_D3D9_H
#define INCLUDED_VIDEODRIVER_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/VideoDriverNull.h"
#include "video/Material.h"
#include "video/LightData.h"
#include "math/matrix4.h"
#include "video/PipelineSettings.h"

#include "core/lxHashMap.h"

#include "StrippedD3D9.h"
#include "video/d3d9/RenderTargetD3D9.h"
#include "video/d3d9/UnknownRefCounted.h"

namespace lux
{

namespace video
{
class RendererD3D9;
class TextureD3D9;

class VideoDriverD3D9 : public VideoDriverNull
{
public:
	VideoDriverD3D9();
	~VideoDriverD3D9();

	void CleanUp();

	void Init(const DriverConfig& config, gui::Window* Window);

	//------------------------------------------------------------------
	virtual StrongRef<Geometry> CreateEmptyGeometry(EPrimitiveType primitiveType = EPrimitiveType::Triangles);
	virtual StrongRef<Geometry> CreateGeometry(
		const VertexFormat& vertexFormat, EHardwareBufferMapping VertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping IndexHWMapping, u32 IndexCount,
		EPrimitiveType primitiveType);

	virtual StrongRef<Geometry> CreateGeometry(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false);

	//------------------------------------------------------------------
	// Textur-Methoden
	bool CheckTextureFormat(ColorFormat format, bool cube);
	StrongRef<Texture> CreateTexture(const math::dimension2du& Size, ColorFormat Format, u32 MipCount, bool isDynamic);
	StrongRef<Texture> CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format);
	StrongRef<CubeTexture> CreateCubeTexture(u32 Size, ColorFormat Format, bool isDynamic);
	bool GetFittingTextureFormat(ColorFormat& format, math::dimension2du& size, bool cube);

	// Cache for auxalarity textures
	/*
	i.e. Dynamic texture for temporary use
	*/

	StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
		const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion,
		core::array<string>* errorList);

	const RendertargetD3D9& GetBackbufferTarget();

	//------------------------------------------------------------------
	EDriverType GetVideoDriverType() const
	{
		return EDriverType::Direct3D9;
	}

	void* GetLowLevelDevice() const
	{
		return m_D3DDevice;
	}
	const D3DCAPS9& GetCaps() const
	{
		return m_Caps;
	}
	const D3DPRESENT_PARAMETERS& GetPresentParams() const
	{
		return m_PresentParams;
	}

	StrongRef<BufferManager> GetBufferManager() const
	{
		return m_BufferManager;
	}

	StrongRef<Renderer> GetRenderer() const
	{
		return m_Renderer;
	}

	IDirect3DSurface9* GetD3D9MatchingDepthBuffer(IDirect3DSurface9* target);
	IDirect3DVertexDeclaration9* GetD3D9VertexDeclaration(const VertexFormat& format);

private:
	class VertexFormat_d3d9
	{
	public:
		VertexFormat_d3d9() {}
		VertexFormat_d3d9(IDirect3DVertexDeclaration9* d3dDecl) :
			m_D3DDeclaration(d3dDecl)
		{
		}

		IDirect3DVertexDeclaration9* GetD3D() const { return m_D3DDeclaration; }

	private:
		UnknownRefCounted<IDirect3DVertexDeclaration9> m_D3DDeclaration;
	};

	class DepthBuffer_d3d9
	{
	public:
		DepthBuffer_d3d9() {}
		DepthBuffer_d3d9(IDirect3DSurface9* surface);
		const math::dimension2du& GetSize() const { return m_Size; }
		IDirect3DSurface9* GetSurface() const { return m_Surface; }

	private:
		math::dimension2du m_Size;
		UnknownRefCounted<IDirect3DSurface9> m_Surface;
	};

private:
	void FillCaps();
	void InitRendertargetData();

	IDirect3DVertexDeclaration9* CreateVertexFormat(const VertexFormat& format);

private:
	RendertargetD3D9 m_BackBufferTarget;
	core::array<DepthBuffer_d3d9> m_DepthBuffers;

	core::array<WeakRef<TextureD3D9>> m_RenderTargets;

	StrongRef<BufferManager> m_BufferManager;
	StrongRef<RendererD3D9> m_Renderer;

	UnknownRefCounted<IDirect3D9> m_D3D;
	UnknownRefCounted<IDirect3DDevice9> m_D3DDevice;

	core::HashMap<VertexFormat, VertexFormat_d3d9> m_VertexFormats;

	bool m_HasStencilBuffer;
	D3DCAPS9 m_Caps;
	int m_Adapter;
	D3DPRESENT_PARAMETERS m_PresentParams;
};

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif