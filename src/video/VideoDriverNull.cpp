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

StrongRef<Texture> VideoDriverNull::CreateFittingTexture(const math::dimension2du& size, ColorFormat format, u32 mipCount, bool isDynamic)
{
	math::dimension2du copy(size);
	if(!GetFittingTextureFormat(format, copy, false))
		throw core::RuntimeException("No matching texture format found");

	return CreateTexture(copy, format, mipCount, isDynamic);
}

StrongRef<CubeTexture> VideoDriverNull::CreateFittingCubeTexture(u32 size, ColorFormat format, bool isDynamic)
{
	math::dimension2du copy(size, size);
	if(!GetFittingTextureFormat(format, copy, true))
		throw core::RuntimeException("No matching texture format found");

	return CreateCubeTexture(copy.width, format, isDynamic);
}

StrongRef<Texture> VideoDriverNull::CreateFittingRendertargetTexture(const math::dimension2du& size, ColorFormat format)
{
	math::dimension2du copy(size);
	if(!GetFittingTextureFormat(format, copy, false))
		throw core::RuntimeException("No matching texture format found");

	return CreateRendertargetTexture(copy, format);
}

u32 VideoDriverNull::GetDeviceCapability(EDriverCaps Capability) const
{
	return m_DriverCaps[(u32)Capability];
}

}
}