#include "video/VideoDriverNull.h"

namespace lux
{
namespace video
{

VideoDriverNull::VideoDriverNull(const VideoDriverInitData& data)
{
	m_Window = data.window;
	m_Config = data.config;
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