#ifndef INCLUDED_LUX_VIDEODRIVER_D3D9_H
#define INCLUDED_LUX_VIDEODRIVER_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/VideoDriverNull.h"

#include "core/lxHashMap.h"

#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"
#include "video/d3d9/RenderTargetD3D9.h"
#include "video/d3d9/AdapterInformationD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"

#include "video/d3d9/RendererD3D9.h"
#include "video/d3d9/HardwareBufferManagerD3D9.h"

namespace lux
{

namespace video
{
class TextureD3D9;

class VideoDriverD3D9 : public VideoDriverNull
{
public:
	VideoDriverD3D9(const video::VideoDriverInitData& data);
	~VideoDriverD3D9();

	void CreateDevice(const DriverConfig& config, HWND window);
	D3DPRESENT_PARAMETERS GeneratePresentParams(const DriverConfig& config);
	bool Reset(const DriverConfig& config);

	//------------------------------------------------------------------
	virtual StrongRef<Geometry> CreateEmptyGeometry(EPrimitiveType primitiveType = EPrimitiveType::Triangles);
	virtual StrongRef<Geometry> CreateGeometry(
		const VertexFormat& vertexFormat, EHardwareBufferMapping VertexHWMapping, int vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping IndexHWMapping, int IndexCount,
		EPrimitiveType primitiveType);

	virtual StrongRef<Geometry> CreateGeometry(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		bool dynamic = false);

	//------------------------------------------------------------------
	// Textur-Methoden
	bool CheckTextureFormat(ColorFormat format, bool cube, bool rendertarget);
	bool GetFittingTextureFormat(ColorFormat& format, math::Dimension2I& size, bool cube, bool rendertarget);

	StrongRef<Texture> CreateTexture(const math::Dimension2I& Size, ColorFormat Format, int MipCount, bool isDynamic);
	StrongRef<Texture> CreateRendertargetTexture(const math::Dimension2I& size, ColorFormat format);
	StrongRef<CubeTexture> CreateCubeTexture(int Size, ColorFormat Format, bool isDynamic);
	StrongRef<CubeTexture> CreateRendertargetCubeTexture(int size, ColorFormat format);

	void AddTextureToList(BaseTexture* tex);

	bool IsShaderSupported(EShaderLanguage lang, int vsMajor, int vsMinor, int psMajor, int psMinor);

	StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		core::StringView VSCode, core::StringView VSEntryPoint, int VSmajorVersion, int VSminorVersion,
		core::StringView PSCode, core::StringView PSEntryPoint, int PSmajorVersion, int PSminorVersion,
		core::Array<core::String>* errorList);

	StrongRef<Shader> CreateFixedFunctionShader(const FixedFunctionParameters& params);

	const RendertargetD3D9& GetBackbufferTarget();

	//------------------------------------------------------------------
	EDeviceState GetDeviceState() const;
	core::Name GetVideoDriverType() const
	{
		return m_Config.adapter->GetDriverType();
	}

	bool HasStencil() const
	{
		return m_HasStencilBuffer;
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

	UnknownRefCounted<IDirect3DSurface9> GetD3D9MatchingDepthBuffer(IDirect3DSurface9* target);
	UnknownRefCounted<IDirect3DVertexDeclaration9> GetD3D9VertexDeclaration(const VertexFormat& format);

private:
	class DepthBuffer_d3d9
	{
	public:
		DepthBuffer_d3d9() {}
		DepthBuffer_d3d9(UnknownRefCounted<IDirect3DSurface9> surface);
		const math::Dimension2I& GetSize() const { return m_Size; }
		IDirect3DSurface9* GetSurface() const { return m_Surface; }

	private:
		math::Dimension2I m_Size;
		UnknownRefCounted<IDirect3DSurface9> m_Surface;
	};

private:
	void FillCaps();
	void InitRendertargetData();

	UnknownRefCounted<IDirect3DVertexDeclaration9> CreateVertexFormat(const VertexFormat& format);

private:
	UnknownRefCounted<IDirect3D9> m_D3D;
	UnknownRefCounted<IDirect3DDevice9> m_D3DDevice;
	HWND m_Window;

	RendertargetD3D9 m_BackBufferTarget;
	core::Array<DepthBuffer_d3d9> m_DepthBuffers;

	core::Array<WeakRef<BaseTexture>> m_Textures;

	StrongRef<BufferManagerD3D9> m_BufferManager;
	StrongRef<RendererD3D9> m_Renderer;
	DeviceStateD3D9 m_DeviceState;

	core::HashMap<VertexFormat, UnknownRefCounted<IDirect3DVertexDeclaration9>> m_VertexFormats;

	bool m_HasStencilBuffer;
	D3DCAPS9 m_Caps;
	StrongRef<AdapterD3D9> m_Adapter;
	D3DPRESENT_PARAMETERS m_PresentParams;
	D3DFORMAT m_AdapterFormat;

	bool m_ReleasedUnmanagedData;
};

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif