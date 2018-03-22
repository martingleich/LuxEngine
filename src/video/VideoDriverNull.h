#ifndef INCLUDED_VIDEO_DRIVER_NULL_H
#define INCLUDED_VIDEO_DRIVER_NULL_H
#include "video/VideoDriver.h"
#include "video/DriverConfig.h"
#include "gui/Window.h"

namespace lux
{
namespace video
{

class VideoDriverNull : public VideoDriver
{
public:
	VideoDriverNull(const VideoDriverInitData& data);

	virtual const DriverConfig& GetConfig() const;
	virtual int GetDeviceCapability(EDriverCaps Capability) const;

protected:
	DriverConfig m_Config;
	int m_DriverCaps[(int)EDriverCaps::EDriverCaps_Count];
};

}
}

#endif // #ifndef INCLUDED_VIDEO_DRIVER_NULL_H