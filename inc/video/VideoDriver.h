#ifndef INCLUDED_LUX_VIDEODRIVER_H
#define INCLUDED_LUX_VIDEODRIVER_H
#include "core/ReferenceCounted.h"
#include "core/ModuleFactory.h"

#include "math/Rect.h"
#include "math/AABBox.h"

#include "video/Color.h"
#include "video/VertexFormat.h"

#include "video/VideoEnums.h"
#include "video/DriverType.h"
#include "video/EDriverCaps.h"
#include "video/DriverConfig.h"

#include "video/HardwareBufferManager.h"
#include "video/TextureStageSettings.h"
#include "video/FixedFunctionShader.h"

namespace lux
{
namespace video
{
class Texture;
class CubeTexture;
class BaseTexture;

class Geometry;

class Renderer;

enum class EDeviceState
{
	OK,
	DeviceLost,
	NotReset,
	Error,
};

class VideoDriverInitData : public core::ModuleInitData
{
public:
	DriverConfig config;
	void* destHandle;
};

class VideoDriver : public ReferenceCounted
{
public:
	virtual ~VideoDriver() {}

	LUX_API static void Initialize(VideoDriver* driver);

	//! Access the global driver
	LUX_API static VideoDriver* Instance();

	//! Destroys the global driver
	LUX_API static void Destroy();

	virtual bool Reset(const DriverConfig& config) = 0;

	//////////////////////////////////////////////////////////////////////////////

	virtual StrongRef<Geometry> CreateEmptyGeometry(
		EPrimitiveType primitiveType = EPrimitiveType::Triangles) = 0;
	virtual StrongRef<Geometry> CreateGeometry(
		const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, int vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, int indexCount,
		EPrimitiveType primitiveType) = 0;
	virtual StrongRef<Geometry> CreateGeometry(
		const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		bool dynamic = false) = 0;

	//////////////////////////////////////////////////////////////////////////////

	virtual bool CheckTextureFormat(ColorFormat format, bool cube, bool rendertarget) = 0;
	virtual bool GetFittingTextureFormat(ColorFormat& format, math::Dimension2I& size, bool cube, bool rendertarget) = 0;

	virtual StrongRef<Texture> CreateTexture(
		const math::Dimension2I& size,
		ColorFormat format = ColorFormat::R8G8B8,
		int mipCount = 0,
		bool isDynamic = false) = 0;

	virtual StrongRef<CubeTexture> CreateCubeTexture(
		int size,
		ColorFormat format = ColorFormat::R8G8B8,
		bool isDynamic = false) = 0;

	virtual StrongRef<Texture> CreateRendertargetTexture(
		const math::Dimension2I& size,
		ColorFormat format) = 0;
	virtual StrongRef<CubeTexture> CreateRendertargetCubeTexture(
		int size,
		ColorFormat format) = 0;

	//////////////////////////////////////////////////////////////////////////////
	//! Creates a new shader from code
	/**
	\throws ShaderCompileException
	*/
	virtual StrongRef<Shader> CreateShader(
		EShaderLanguage language,
		const char* VSCode, const char* VSEntryPoint, int VSLength, int VSmajorVersion, int VSminorVersion,
		const char* PSCode, const char* PSEntryPoint, int PSLength, int PSmajorVersion, int PSminorVersion,
		core::Array<core::String>* errorList) = 0;

	//! Create a new fixed function shader.
	virtual StrongRef<Shader> CreateFixedFunctionShader(const FixedFunctionParameters& params) = 0;

	//! Checks if some shader language and version is supported
	virtual bool IsShaderSupported(EShaderLanguage lang, int vsMajor, int vsMinor, int psMajor, int psMinor) = 0;

	//////////////////////////////////////////////////////////////////////////////

	virtual const DriverConfig& GetConfig() const = 0;

	virtual StrongRef<BufferManager> GetBufferManager() const = 0;
	virtual StrongRef<Renderer> GetRenderer() const = 0;

	virtual EDeviceState GetDeviceState() const = 0;

	virtual int GetDeviceCapability(EDriverCaps capability) const = 0;
	virtual const core::String& GetVideoDriverType() const = 0;
	virtual void* GetLowLevelDevice() const = 0;
};

} // namespace video
} // namespace lux

#endif