#ifndef INCLUDED_VIDEODRIVER_D3D9_H
#define INCLUDED_VIDEODRIVER_D3D9_H
#include "core/ReferableFactory.h"
#include "video/VideoDriver.h"
#include "video/Material.h"
#include "video/LightData.h"
#include "math/matrix4.h"
#include "video/PipelineSettings.h"

#include "core/lxHashMap.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"

namespace lux
{

namespace core
{
class Timer;
}

namespace video
{

class VideoDriverD3D9 : public VideoDriver
{
public:
	VideoDriverD3D9(core::Timer* timer, core::ReferableFactory* refFactory);
	~VideoDriverD3D9()
	{
		Exit();
	}

	bool Init(const DriverConfig& config, gui::Window* Window);
	void Exit();

	bool BeginScene(bool color, bool zbuffer);
	bool EndScene();
	bool Present();

	void SetClearValues(Color color, float depth);

	bool SetRendertarget(Texture* texture);
	Texture* GetRendertarget();

	//------------------------------------------------------------------
	// Materialien
	void Set3DMaterial(const Material& material);
	const Material&   Get3DMaterial() const;

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
	// Lichter
	size_t AddLight(const LightData& light);
	const LightData& GetLight(size_t index);
	void EnableLight(size_t index, bool bTurnOn);
	size_t GetLightCount() const;
	void DeleteAllLights();
	size_t GetMaximalLightCount() const;

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
	bool CheckTextureFormat(ColorFormat format, bool alpha, bool cube);
	StrongRef<Texture> CreateTexture(const math::dimension2du& Size, ColorFormat Format, u32 MipCount, bool Alpha, bool isDynamic);
	StrongRef<Texture> CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format, bool alpha);
	StrongRef<CubeTexture> CreateCubeTexture(u32 Size, ColorFormat Format, bool Alpha, bool isDynamic);

	// Cache for auxalarity textures
	/*
	i.e. Dynamic texture for temporary use
	*/
	StrongRef<Shader> CreateShader(
		const char* VSCode, const char* VSEntryPoint, u32 VSLength, EVertexShaderType VSType,
		const char* PSCode, const char* PSEntryPoint, u32 PSLength, EPixelShaderType PSType);

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
	virtual void        SetAmbient(Color ambient);
	virtual Color        GetAmbient() const;

	u32 GetDeviceCapability(EDriverCaps Capability) const
	{
		return m_DriverCaps[(u32)Capability];
	}

	//------------------------------------------------------------------
	// Inline-Methoden
	const DriverConfig& GetConfig() const
	{
		return m_Config;
	}

	EVideoDriver GetVideoDriverType() const
	{
		return EVD_DIRECT9;
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
		return m_CurrentRendertarget.GetSize();
	}
	bool GetPresentResult() const
	{
		return m_PresentResult;
	}

	StrongRef<RenderStatistics> GetRenderStatistics() const
	{
		return m_RenderStatistics;
	}
	StrongRef<scene::SceneValues> GetSceneValues() const
	{
		return m_SceneValues;
	}
	StrongRef<BufferManager> GetBufferManager() const
	{
		return m_BufferManager;
	}

	void SetDefaultRenderer(MaterialRenderer* r)
	{
		m_SolidRenderer = r;
	}

	static BYTE GetD3DUsage(VertexElement::EUsage usage);
	static D3DFORMAT GetD3DFormat(ColorFormat Format, bool Alpha);
	static ColorFormat GetLuxFormat(D3DFORMAT Format);
	static u32 GetBitsPerPixel(D3DFORMAT Format);
	static u32 GetD3DBlend(EBlendFactor factor);
	static u32 GetD3DBlendFunc(EBlendOperator Op);
	static u32 GetD3DRepeatMode(ETextureRepeat repeat);
	static u32 GetD3DDeclType(VertexElement::EType type);
	static D3DCOLORVALUE SColorToD3DColor(const Colorf& color);

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

	class Rendertarget_d3d9
	{
	public:
		Rendertarget_d3d9();
		Rendertarget_d3d9(Texture* texture);
		Rendertarget_d3d9(IDirect3DSurface9* surface);
		const math::dimension2du& GetSize() const;
		StrongRef<Texture> GetTexture();
		IDirect3DSurface9* GetSurface();

	private:
		StrongRef<Texture> m_Texture;
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
	Rendertarget_d3d9 m_CurrentRendertarget;

	WeakRef<Texture> m_PreviousRendertarget;
	IDirect3DSurface9* m_PreviousRendertargetSurface;

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

	StrongRef<core::Timer> m_Timer;
	StrongRef<RenderStatistics> m_RenderStatistics;
	StrongRef<scene::SceneValues> m_SceneValues;
	StrongRef<BufferManager> m_BufferManager;

	FogInformation_d3d9 m_Fog;

	Color m_ClearColor;
	float m_ClearDepth;

	size_t m_LastSetLight;
	core::array<LightData> m_LightList;

	IDirect3D9* m_D3D;
	IDirect3DDevice9* m_D3DDevice;
	bool m_PresentResult;

	core::array<StrongRef<BaseTexture>> m_Textures;

	math::matrix4 m_Transforms[ETS_COUNT];
	math::matrix4 m_2DTranform;

	Color m_AmbientColor;

	VertexFormat m_CurrentVertexFormat;
	core::HashMap<VertexFormat, VertexFormat_d3d9> m_VertexFormats;

	core::array<PipelineOverwrite> m_PipelineOverwrites;

	DriverConfig m_Config;
	u32 m_DriverCaps[(u32)EDriverCaps::EDriverCaps_Count];
	bool m_HasStencilBuffer;
	D3DCAPS9 m_Caps;
	int m_Adapter;
	D3DPRESENT_PARAMETERS m_PresentParams;

	video::MaterialRenderer* m_SolidRenderer;

	core::ReferableFactory* m_RefFactory;
};

}

}


#endif // LUX_COMPILE_WITH_D3D9

#endif