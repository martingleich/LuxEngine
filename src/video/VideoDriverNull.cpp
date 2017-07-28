#include "video/VideoDriverNull.h"
namespace lux
{
namespace video
{

void VideoDriverNull::Init(const DriverConfig& config, gui::Window* window)
{
	LUX_UNUSED(window);
	m_Config = config;
}

EDriverType VideoDriverNull::GetVideoDriverType() const
{
	return EDriverType::Null;
}

const DriverConfig& VideoDriverNull::GetConfig() const
{
	return m_Config;
}

u32 VideoDriverNull::GetDeviceCapability(EDriverCaps Capability) const
{
	return m_DriverCaps[(u32)Capability];
}

}
}