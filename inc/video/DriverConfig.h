#ifndef INCLUDED_DRIVER_CONFIG
#define INCLUDED_DRIVER_CONFIG
#include "core/LuxBase.h"
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/ModuleFactory.h"

#include "math/Dimension2.h"

#include "video/DriverType.h"
#include "video/VideoEnums.h"
#include "video/ColorFormat.h"

#include "core/lxArray.h"

namespace lux
{
namespace video
{

class DriverConfig;

struct DisplayMode
{
	u32 width;
	u32 height;
	u32 refreshRate;
	ColorFormat format;
};

class Adapter;

class DriverConfig
{
public:
	StrongRef<Adapter> adapter;

	DisplayMode display;

	ColorFormat backBufferFormat = ColorFormat::A8R8G8B8;

	ZStencilFormat zsFormat;

	u32 multiSamples = 0;
	u32 multiQuality = 0;

	bool windowed = true;
	bool vSync = false;
};

class Adapter : public ReferenceCounted
{
public:
	virtual ~Adapter() {}

	virtual const String& GetName() const = 0;
	virtual u32 GetVendor() const = 0;
	virtual u32 GetDevice() const = 0;
	virtual const String& GetDriverType() const = 0;

	virtual core::Array<DisplayMode> GenerateDisplayModes(bool windowed) = 0;
	virtual core::Array<ColorFormat> GenerateBackbufferFormats(const DisplayMode& mode, bool windowed) = 0;
	virtual core::Array<ZStencilFormat> GenerateZStencilFormats(const DisplayMode& mode, bool windowed, ColorFormat backBuffer) = 0;
	virtual core::Array<u32> GenerateMultisampleLevels(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat) = 0;
	virtual u32 GetNumMultisampleQualities(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, u32 level) = 0;

	LUX_API virtual u32 GetMaxMultisampleLevel(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat);
	LUX_API virtual bool GetMatchingMode(DisplayMode& outMode, const math::Dimension2U& minRes, bool windowed, u32 minRefresh = 0);
	LUX_API virtual bool GetMatchingBackbuffer(ColorFormat& outFormat, const DisplayMode& mode, bool windowed, bool use16Bit = false);
	LUX_API virtual bool GetMatchingZStencil(ZStencilFormat& outFormat, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, u32 minDepth = 0, u32 minStencil = 0);
	LUX_API virtual bool GetMatchingMultisample(u32& outLevel, u32& outQuality, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, u32 minSamples, u32 minQuality);

	//! Generate a simple config file for the given settings
	/**
	\param [out] outConfig the generated config data.
	\param minRes The minimal resolution of the backbuffer, the resulting
		resolution will be bigger or equal on all axes.
	\param windowed Windowed or fullscreen
	\param vSync Is vSync enabled
	\param backBuffer16Bit Should the backbuffer use 16 bit colors
	\param minDepth The minimal number of depth bits
	\param minStencil The minimal number of stencil bits
	\param multiSample The level of multisampling with 0 meaning no multisampling
		and 10 meaning as much multisampling as possible
	\return Was the config generated successfully.
	*/
	LUX_API virtual bool GenerateConfig(
		video::DriverConfig& outConfig,
		const math::Dimension2U& minRes,
		bool windowed, bool vSync,
		bool backBuffer16Bit,
		u32 minDepth,
		u32 minStencil,
		int multiSample);
};

struct AdapterListInitData : public core::ModuleInitData
{
};

class AdapterList : public ReferenceCounted
{
public:
	virtual ~AdapterList() {}
	virtual u32 GetAdapterCount() const = 0;
	virtual StrongRef<Adapter> GetAdapter(u32 idx) const = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_DRIVER_CONFIG