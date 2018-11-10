#ifndef INCLUDED_LUX_VIDEO_DRIVER_NULL_H
#define INCLUDED_LUX_VIDEO_DRIVER_NULL_H
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

	virtual const DriverConfig& GetConfig() const override;
	virtual int GetDeviceCapability(EDriverCaps Capability) const override;

protected:
	DriverConfig m_Config;
	int m_DriverCaps[(int)EDriverCaps::EDriverCaps_Count];
};

}
}

#endif // #ifndef INCLUDED_LUX_VIDEO_DRIVER_NULL_H