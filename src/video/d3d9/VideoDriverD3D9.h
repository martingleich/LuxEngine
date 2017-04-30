#ifndef INCLUDED_VIDEODRIVER_D3D9_H
#define INCLUDED_VIDEODRIVER_D3D9_H
#include "video/VideoDriverNull.h"
#include "video/Material.h"
#include "video/LightData.h"
#include "math/matrix4.h"
#include "video/PipelineSettings.h"

#include "core/lxHashMap.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"
#include "video/d3d9/RenderTargetD3D9.h"

namespace lux
{

namespace video
{

class VideoDriverD3D9 : public VideoDriverNull
{
public:
	VideoDriverD3D9(core::ReferableFactory* refFactory);
	~VideoDriverD3D9();

	bool Init(const DriverConfig& config, gui::Window* Window);

	bool BeginScene(bool cleaColor, bool clearZ,
		video::Color color, float z);
	bool EndScene();
	bool Present();

	bool SetRenderTarget(const RenderTarget& target);
	const RenderTarget& GetRenderTarget();

	//------------------------------------------------------------------
	// Materialien
	void Set3DMaterial(const Material& material);
	const Material& Get3DMaterial() const;

	void Set2DMaterial(const Material& material);
	const Material& Get2DMaterial() const;

	void SetRenderstates3DMode(bool useMaterial = true);
	void SetRenderstates2DMode(bool useMaterial = true, bool alpha = false, bool useAlphaChannel = false);

	void ResetAllRenderstates();

	void PushPipelineOverwrite(const PipelineOverwrite& over);
	void PopPipelineOverwrite();

	void EnablePipeline(const PipelineSettings& settings, bool resetAll = false);
	void SetTextureLayer(const MaterialLayer& layer, u32 textureLayer, bool resetAll = false);

	//------------------------------------------------------------------
	virtual StrongRef<SubMesh> CreateSubMesh(
		const VertexFormat& vertexFormat, EHardwareBufferMapping VertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping IndexHWMapping, u32 IndexCount,
		EPrimitiveType primitiveType);

	virtual StrongRef<SubMesh> CreateSubMesh(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		bool Dynamic = false,
		EPrimitiveType primitiveType = EPT_TRIANGLES,
		u32 primitiveCount = 0);

	//------------------------------------------------------------------
	// Textur-Methoden
	bool CheckTextureFormat(ColorFormat format, bool cube);
	StrongRef<Texture> CreateTexture(const math::dimension2du& Size, ColorFormat Format, u32 MipCount, bool isDynamic);
	StrongRef<Texture> CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format);
	StrongRef<CubeTexture> CreateCubeTexture(u32 Size, ColorFormat Format, bool isDynamic);

	// Cache for auxalarity textures
	/*
	i.e. Dynamic texture for temporary use
	*/

	StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
		const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion);

	//------------------------------------------------------------------
	// Renderfunktionen
	bool Draw3DPrimitiveList(EPrimitiveType primitiveType,
		u32 primitveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType);

	bool Draw2DPrimitiveList(EPrimitiveType primitiveType,
		u32 primitveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType);

	bool DrawPrimitiveList(EPrimitiveType primitiveType,
		u32 primitveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType,
		bool is3D);

	bool Draw3DLine(const math::vector3f& start, const math::vector3f& end, Color colorStart, Color colorEnd, bool disableZ);
	bool Draw3DBox(const math::aabbox3df& box, Color color);
	//bool Draw3DTriangle(const math::triangle3df& Triangle, Color color = Color::White);

	bool Draw2DImage(Texture* texture, const math::vector2i& position, const math::recti* clip = nullptr, const video::Material* material = nullptr);

	bool Draw2DImage(Texture* texture, const math::recti& DstRect, const math::rectf& SrcRect, Color color = Color::White, bool UseAlpha = false, const math::recti* clip = nullptr, const video::Material* material = nullptr);

	bool Draw2DRectangle(const math::recti& rect, Color color = Color::White, const math::recti* clip = nullptr, const video::Material* material = nullptr);

	bool Draw2DRectangle(const math::recti& rect, Color LeftUpColor, Color RightUpColor, Color LeftDownColor, Color RightDownColor, const math::recti* clip = nullptr, const video::Material* material = nullptr);

	bool Draw2DLine(const math::vector2i& start, const math::vector2i& end,
		Color colorStart, Color colorEnd, const math::recti* clip = nullptr);
	void DrawSubMesh(const SubMesh* subMesh, u32 primitveCount = -1);

	void SetActiveTexture(u32 stage, BaseTexture* texture);                            // Setzt eine Textur
	BaseTexture* GetActiveTexture(u32 stage) const;                                            // Fragt eine Textur ab

	virtual void SetTransform(ETransformState transform, const math::matrix4& matrix);    // Setzt eine Transformationsmatrix
	virtual const math::matrix4& GetTransform(ETransformState Transforn) const;                        // Fragt eine Transformationsmatrix ab
	virtual void Set2DTransform(const math::matrix4& matrix);
	virtual void Use2DTransform(bool Use);

	virtual void SetVertexFormat(const VertexFormat& vertexFormat, bool reset = false);                    // Setzt das Vertexformat
	virtual const VertexFormat& GetVertexFormat() const;

	void SetFog(Color color = Color(0),
		EFogType FogType = EFT_LINEAR,
		float start = 50.0f,
		float end = 100.0f,
		float density = 0.01f,
		bool pixelFog = false,
		bool rangeFog = false);

	void GetFog(Color* color = nullptr,
		EFogType* FogType = nullptr,
		float* start = nullptr,
		float* end = nullptr,
		float* density = nullptr,
		bool* pixelFog = nullptr,
		bool* rangeFog = nullptr) const;

	bool AddLight(const LightData& light);
	void ClearLights();

	//------------------------------------------------------------------
	EVideoDriver GetVideoDriverType() const
	{
		return EVideoDriver::Direct3D9;
	}

	void* GetDevice() const
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
	const math::dimension2du& GetScreenSize() const
	{
		return m_CurrentRenderTarget.GetSize();
	}

	bool GetPresentResult() const
	{
		return m_PresentResult;
	}

	StrongRef<BufferManager> GetBufferManager() const
	{
		return m_BufferManager;
	}
	void SetDefaultRenderer(MaterialRenderer* r)
	{
		m_SolidRenderer = r;
	}

private:
	class VertexFormat_d3d9
	{
	public:
		VertexFormat_d3d9();
		VertexFormat_d3d9(IDirect3DVertexDeclaration9* d3dDecl);
		VertexFormat_d3d9(const VertexFormat_d3d9& other);
		VertexFormat_d3d9& operator=(const VertexFormat_d3d9& other);
		VertexFormat_d3d9(VertexFormat_d3d9&& old);
		VertexFormat_d3d9& operator=(VertexFormat_d3d9&& old);
		~VertexFormat_d3d9();
		IDirect3DVertexDeclaration9* GetD3D() const;

	private:
		IDirect3DVertexDeclaration9* m_D3DDeclaration;
	};

	class DepthBuffer_d3d9
	{
	public:
		DepthBuffer_d3d9();
		DepthBuffer_d3d9(IDirect3DSurface9* surface);
		DepthBuffer_d3d9& operator=(const DepthBuffer_d3d9& other);
		~DepthBuffer_d3d9();
		const math::dimension2du& GetSize() const;
		IDirect3DSurface9* GetSurface() const;

	private:
		math::dimension2du m_Size;
		IDirect3DSurface9* m_Surface;
	};

	struct FogInformation_d3d9
	{
		Color color;
		EFogType type;
		float start;
		float end;
		float density;
		bool pixel;
		bool range;

		bool isDirty;
	};

private:
	void FillCaps();
	bool InitRendertargetData();

	IDirect3DSurface9* GetMatchingDepthBuffer(IDirect3DSurface9* target);
	IDirect3DVertexDeclaration9* CreateVertexFormat(const VertexFormat& format);

private:
	Rendertarget_d3d9 m_CurrentRenderTarget;
	Rendertarget_d3d9 m_BackBufferTarget;

	core::array<DepthBuffer_d3d9> m_DepthBuffers;

	bool m_ResetRenderstates;
	PipelineSettings m_CurrentPipeline;
	Material m_CurrentMaterial;
	bool m_3DTransformsChanged;
	bool m_2DTransformChanged;
	bool m_Use2DTransform;

	Material m_3DMaterial;
	Material m_2DMaterial;
	PipelineSettings m_Pipeline;

	ERenderMode m_CurrentRendermode;

	StrongRef<BufferManager> m_BufferManager;
	FogInformation_d3d9 m_Fog;

	IDirect3D9* m_D3D;
	IDirect3DDevice9* m_D3DDevice;
	bool m_PresentResult;

	core::array<StrongRef<BaseTexture>> m_Textures;

	math::matrix4 m_Transforms[ETS_COUNT];
	math::matrix4 m_2DTranform;

	VertexFormat m_CurrentVertexFormat;
	core::HashMap<VertexFormat, VertexFormat_d3d9> m_VertexFormats;

	core::array<PipelineOverwrite> m_PipelineOverwrites;

	bool m_HasStencilBuffer;
	D3DCAPS9 m_Caps;
	int m_Adapter;
	D3DPRESENT_PARAMETERS m_PresentParams;

	video::MaterialRenderer* m_SolidRenderer;
};

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif