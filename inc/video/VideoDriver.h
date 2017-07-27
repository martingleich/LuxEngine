#ifndef INCLUDED_VIDEODRIVER_H
#define INCLUDED_VIDEODRIVER_H
#include "core/ReferenceCounted.h"

#include "math/Rect.h"
#include "math/AABBox.h"

#include "video/Color.h"
#include "video/VertexFormats.h"

#include "video/EPrimitiveType.h"
#include "video/EShaderTypes.h"
#include "video/DriverConfig.h"
#include "video/EDriverCaps.h"

#include "video/HardwareBufferConstants.h"
#include "video/HardwareBufferManager.h"

namespace lux
{
namespace gui
{
class Window;
}

namespace video
{
class Shader;
class Texture;
class CubeTexture;
class BaseTexture;

class Geometry;
class RenderStatistics;

class Renderer;

class VideoDriver : public ReferenceCounted
{
public:
	virtual ~VideoDriver() {}

	//! Initialize the global driver
	LUX_API static void Initialize(VideoDriver* driver = nullptr);

	//! Access the global driver
	LUX_API static VideoDriver* Instance();

	//! Destroys the global driver
	LUX_API static void Destroy();

	virtual StrongRef<Geometry> CreateEmptyGeometry(EPrimitiveType primitiveType = EPrimitiveType::Triangles) = 0;
	virtual StrongRef<Geometry> CreateGeometry(
		const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
		EPrimitiveType primitiveType) = 0;
	virtual StrongRef<Geometry> CreateGeometry(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false) = 0;

	virtual bool CheckTextureFormat(ColorFormat format, bool cube) = 0;

	virtual bool GetFittingTextureFormat(ColorFormat& format, math::Dimension2U& size, bool cube) = 0;

	virtual StrongRef<Texture> CreateFittingTexture(const math::Dimension2U& size, ColorFormat format=ColorFormat::R8G8B8, u32 mipCount=0, bool isDynamic=false) = 0;
	virtual StrongRef<CubeTexture> CreateFittingCubeTexture(u32 size, ColorFormat format=ColorFormat::R8G8B8, bool isDynamic=false) = 0;
	virtual StrongRef<Texture> CreateFittingRendertargetTexture(const math::Dimension2U& size, ColorFormat format) = 0;

	virtual StrongRef<Texture> CreateTexture(const math::Dimension2U& size, ColorFormat format=ColorFormat::R8G8B8, u32 mipCount=0, bool isDynamic=false) = 0;
	virtual StrongRef<CubeTexture> CreateCubeTexture(u32 size, ColorFormat format=ColorFormat::R8G8B8, bool isDynamic=false) = 0;
	virtual StrongRef<Texture> CreateRendertargetTexture(const math::Dimension2U& size, ColorFormat format) = 0;

	//! Creates a new shader from code
	/**
	\throws ShaderCompileException
	*/
	virtual StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		const char* VSCode, const char* VSEntryPoint, u32 VSLength, int VSmajorVersion, int VSminorVersion,
		const char* PSCode, const char* PSEntryPoint, u32 PSLength, int PSmajorVersion, int PSminorVersion,
		core::Array<String>* errorList) = 0;

	virtual EDriverType GetVideoDriverType() const = 0;
	virtual const DriverConfig& GetConfig() const = 0;
	virtual u32 GetDeviceCapability(EDriverCaps Capability) const = 0;
	virtual StrongRef<BufferManager> GetBufferManager() const = 0;
	virtual void* GetLowLevelDevice() const = 0;

	virtual StrongRef<Renderer> GetRenderer() const = 0;
};

} // namespace video
} // namespace lux

#endif