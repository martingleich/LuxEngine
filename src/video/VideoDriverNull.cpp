#include "video/VideoDriverNull.h"

namespace lux
{
namespace video
{

VideoDriverNull::VideoDriverNull(const VideoDriverInitData& data)
{
	m_Config = data.config;
}

const DriverConfig& VideoDriverNull::GetConfig() const
{
	return m_Config;
}

int VideoDriverNull::GetDeviceCapability(EDriverCaps Capability) const
{
	return m_DriverCaps[(int)Capability];
}

}
}