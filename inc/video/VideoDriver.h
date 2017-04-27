#ifndef INCLUDED_VIDEODRIVER_H
#define INCLUDED_VIDEODRIVER_H
#include "core/ReferenceCounted.h"

#include "math/rect.h"
#include "math/aabbox3d.h"

#include "video/Color.h"
#include "video/VertexFormats.h"
#include "EPrimitiveType.h"
#include "EShaderTypes.h"

#include "HardwareBufferConstants.h"

namespace lux
{

namespace math
{
class matrix4;
}

namespace gui
{
class Window;
}

namespace scene
{
class SceneValues;
}

namespace video
{
class Shader;
class Texture;
class CubeTexture;
class BaseTexture;

class SubMesh;
class RenderStatistics;
class BufferManager;
class Material;
class MaterialLayer;
class LightData;
class PipelineSettings;
class PipelineOverwrite;
}


namespace video
{

//! Diffrent types of usable video drivers
enum EVideoDriver
{
	//! The direct3D 9 video driver
	EVD_DIRECT9 = 0,

	//! The null driver
	EVD_NULL,
};

struct DriverConfig
{
	EVideoDriver driverType;

	u32 width;
	u32 height;

	u32 zBits;
	bool backbuffer16Bit;

	u32 multiSampling;

	bool windowed;
	bool vSync;
	bool stencil;

	bool pureSoftware;

	DriverConfig() :
		driverType(EVD_DIRECT9),
		width(800),
		height(600),
		zBits(16),
		backbuffer16Bit(false),
		multiSampling(0),
		windowed(false),
		vSync(true),
		stencil(false),
		pureSoftware(false)
	{}

	inline static DriverConfig WindowDirect3D(
		u32 _width,
		u32 _height,
		bool vSync=true)
	{
		DriverConfig config;
		config.width = _width;
		config.height = _height;
		config.vSync = vSync;
		config.driverType = EVD_DIRECT9;
		config.windowed = true;

		return config;
	}
};

enum EFogType
{
	// Expontenieller Nebel
	EFT_EXP = 1,
	// Quadratisch expontenieller Nebel
	EFT_EXP2,
	// Linearer Nebel
	EFT_LINEAR
};

enum ETransformState
{
	ETS_VIEW = 0,
	ETS_PROJECTION,
	ETS_WORLD,
	ETS_COUNT
};

enum ERenderMode
{
	ERM_3D = 0,
	ERM_2D,
	ERM_NONE
};

enum class EDriverCaps
{
	//! The maximum number of primitives per DrawCall
	MaxPrimitives = 0,
	//! The maximum number of parallel used streams
	MaxStreams,
	//! The maximum texture width
	MaxTextureWidth,
	//! The maximal texture height
	MaxTextureHeight,
	//! Textures must be a power of two
	TexturesPowerOfTwoOnly,
	//! Textures must be square
	TextureSquareOnly,
	//! Maximum number of parallel texture
	MaxSimultaneousTextures,

	EDriverCaps_Count
};

class VideoDriver : public ReferenceCounted
{
public:
	virtual ~VideoDriver()
	{
	}

	virtual bool Init(const DriverConfig& config, gui::Window* window) = 0;
	virtual void Exit() = 0;
	virtual bool Present() = 0;
	virtual bool BeginScene(bool ClearColor, bool ClearZBuffer) = 0;
	virtual bool EndScene() = 0;

	virtual void SetClearValues(Color color, float depth) = 0;

	virtual size_t AddLight(const LightData& light) = 0;
	virtual const LightData& GetLight(size_t index) = 0;
	virtual void EnableLight(size_t index, bool turnOn) = 0;
	virtual size_t  GetLightCount() const = 0;
	virtual void DeleteAllLights() = 0;
	virtual size_t GetMaximalLightCount() const = 0;

	virtual bool SetRendertarget(Texture* texture) = 0;
	virtual Texture* GetRendertarget() = 0;

	virtual void Set3DMaterial(const Material& material) = 0;
	virtual const Material& Get3DMaterial() const = 0;

	virtual void Set2DMaterial(const Material& material) = 0;
	virtual const Material& Get2DMaterial() const = 0;

	virtual void SetRenderstates3DMode(bool useMaterial = true) = 0;
	virtual void SetRenderstates2DMode(bool useMaterial = true, bool alpha = false, bool alphaChannel = false) = 0;
	virtual void ResetAllRenderstates() = 0;

	virtual void PushPipelineOverwrite(const PipelineOverwrite& over) = 0;
	virtual void PopPipelineOverwrite() = 0;

	virtual void EnablePipeline(const PipelineSettings& settings, bool resetAll = false) = 0;
	virtual void SetTextureLayer(const MaterialLayer& Layer, u32 textureLayer, bool resetAll = false) = 0;

	virtual StrongRef<SubMesh> CreateSubMesh(const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
		EPrimitiveType primitiveType) = 0;

	virtual StrongRef<SubMesh> CreateSubMesh(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		bool dynamic = false,
		EPrimitiveType primitiveType = EPT_TRIANGLES,
		u32 primitiveCount = 0) = 0;

	virtual bool CheckTextureFormat(ColorFormat format, bool alpha, bool cube) = 0;
	virtual StrongRef<Texture> CreateTexture(const math::dimension2du& size, ColorFormat format, u32 mipCount, bool alpha, bool isDynamic) = 0;
	virtual StrongRef<CubeTexture> CreateCubeTexture(u32 size, ColorFormat format, bool alpha, bool isDynamic) = 0;
	virtual StrongRef<Texture> CreateRendertargetTexture(const math::dimension2du& size, ColorFormat format, bool alpha) = 0;

	virtual StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
		const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion) = 0;

	virtual bool Draw3DPrimitiveList(EPrimitiveType primitiveType,
		u32 primitiveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType) = 0;

	virtual bool Draw2DPrimitiveList(EPrimitiveType primitiveType,
		u32 primitiveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType) = 0;

	virtual bool DrawPrimitiveList(EPrimitiveType primitiveType,
		u32 primitveCount,
		const void* vertices,
		u32 vertexCount,
		const VertexFormat& vertexFormat,
		const void* indices,
		EIndexFormat indexType,
		bool is3D) = 0;

	virtual bool Draw3DLine(const math::vector3f& start, const math::vector3f& end, Color colorStart, Color colorEnd, bool disableZ=false) = 0;
	virtual bool Draw3DBox(const math::aabbox3df& box, Color color) = 0;
	virtual bool Draw2DImage(Texture* texture, const math::vector2i& position, const math::recti* clip = nullptr, const video::Material* material = nullptr) = 0;
	virtual bool Draw2DImage(Texture* texture, const math::recti& DstRect, const math::rectf& SrcRect, Color color = Color::White, bool UseAlpha = false, const math::recti* clip = nullptr, const video::Material* material = nullptr) = 0;
	virtual bool Draw2DRectangle(const math::recti& rect, Color color = Color::White, const math::recti* clip = nullptr, const video::Material* material = nullptr) = 0;
	virtual bool Draw2DRectangle(const math::recti& rect, Color LeftUpColor, Color RightUpColor, Color LeftDownColor, Color RightDownColor, const math::recti* clip = nullptr, const video::Material* material = nullptr) = 0;
	virtual bool Draw2DLine(const math::vector2i& start, const math::vector2i& end, Color colorStart, Color colorEnd, const math::recti* clip = nullptr) = 0;
	virtual void DrawSubMesh(const SubMesh* subMesh, u32 PrimtiveCount = -1) = 0;

	virtual void SetActiveTexture(u32 stage, BaseTexture* texture) = 0;
	virtual BaseTexture* GetActiveTexture(u32 stage) const = 0;

	virtual void SetTransform(ETransformState transform, const math::matrix4& matrix) = 0;
	virtual const math::matrix4& GetTransform(ETransformState Transforn) const = 0;

	virtual void Set2DTransform(const math::matrix4& matrix) = 0;
	virtual void Use2DTransform(bool Use) = 0;

	//virtual void SetVertexFormat(const VertexFormat& vertexFormat, bool reset = false) = 0;
	virtual void SetVertexFormat(const VertexFormat& format, bool reset = false) = 0;

	virtual void SetFog(Color color = Color(0),
		EFogType FogType = EFT_LINEAR,
		float start = 50.0f,
		float end = 100.0f,
		float density = 0.01f,
		bool pixelFog = false,
		bool rangeFog = false) = 0;

	virtual void GetFog(Color* color = nullptr,
		EFogType* FogType = nullptr,
		float* start = nullptr,
		float* end = nullptr,
		float* density = nullptr,
		bool* pixelFog = nullptr,
		bool* rangeFog = nullptr) const = 0;

	virtual void SetAmbient(Color ambient) = 0;
	virtual Color GetAmbient() const = 0;

	virtual void* GetDevice() const = 0;
	virtual EVideoDriver GetVideoDriverType() const = 0;

	virtual const VertexFormat& GetVertexFormat() const = 0;
	virtual bool GetPresentResult() const = 0;

	virtual const DriverConfig& GetConfig() const = 0;
	virtual const math::dimension2du& GetScreenSize() const = 0;

	virtual u32 GetDeviceCapability(EDriverCaps Capability) const = 0;

	virtual StrongRef<RenderStatistics> GetRenderStatistics() const = 0;
	virtual StrongRef<BufferManager> GetBufferManager() const = 0;
	virtual StrongRef<scene::SceneValues> GetSceneValues() const = 0;
};

}
}

#endif