#ifndef INCLUDED_VIDEO_DRIVER_NULL_H
#define INCLUDED_VIDEO_DRIVER_NULL_H
#include "video/VideoDriver.h"

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
	
	virtual u32 GetDeviceCapability(EDriverCaps Capability) const;

protected:
	DriverConfig m_Config;
	u32 m_DriverCaps[(u32)EDriverCaps::EDriverCaps_Count];
};

}
}

#endif // #ifndef INCLUDED_VIDEO_DRIVER_NULL_H