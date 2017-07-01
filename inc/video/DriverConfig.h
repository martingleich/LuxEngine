#ifndef INCLUDED_DRIVER_CONFIG
#define INCLUDED_DRIVER_CONFIG
#include "core/LuxBase.h"
#include "video/EDriverType.h"

namespace lux
{
namespace video
{
struct DriverConfig
{
	EDriverType driverType;

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
		driverType(EDriverType::Null),
		width(800),
		height(600),
		zBits(16),
		backbuffer16Bit(false),
		multiSampling(0),
		windowed(false),
		vSync(true),
		stencil(false),
		pureSoftware(false)
	{
	}

	inline static DriverConfig WindowedDefault(
		u32 _width,
		u32 _height,
		bool vSync = true)
	{
		DriverConfig config;
		config.width = _width;
		config.height = _height;
		config.vSync = vSync;
		config.driverType = EDriverType::Direct3D9;
		config.windowed = true;

		return config;
	}

	inline static DriverConfig WindowedDirect3D(
		u32 _width,
		u32 _height,
		bool vSync = true)
	{
		DriverConfig config;
		config.width = _width;
		config.height = _height;
		config.vSync = vSync;
		config.driverType = EDriverType::Direct3D9;
		config.windowed = true;

		return config;
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_DRIVER_CONFIG