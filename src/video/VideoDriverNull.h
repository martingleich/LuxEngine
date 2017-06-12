#ifndef INCLUDED_VIDEO_DRIVER_NULL_H
#define INCLUDED_VIDEO_DRIVER_NULL_H
#include "video/VideoDriver.h"
#include "core/ReferableFactory.h"
#include "video/RenderStatistics.h"

namespace lux
{
namespace video
{

class VideoDriverNull : public VideoDriver
{
public:
	virtual void Init(const DriverConfig& config, gui::Window* Window);

	virtual EDriverType GetVideoDriverType() const;
	virtual const DriverConfig& GetConfig() const;
	
	virtual StrongRef<Texture> CreateFittingTexture(const math::dimension2du& size, ColorFormat format=ColorFormat::R8G8B8, u32 mipCount=0, bool isDynamic=false);
	virtual StrongRef<CubeTexture> CreateFittingCubeTexture(u32 size, ColorFormat format=ColorFormat::R8G8B8, bool isDynamic=false);
	virtual StrongRef<Texture> CreateFittingRendertargetTexture(const math::dimension2du& size, ColorFormat format);

	virtual u32 GetDeviceCapability(EDriverCaps Capability) const;

protected:
	DriverConfig m_Config;
	u32 m_DriverCaps[(u32)EDriverCaps::EDriverCaps_Count];
};

}
}

#endif // #ifndef INCLUDED_VIDEO_DRIVER_NULL_H