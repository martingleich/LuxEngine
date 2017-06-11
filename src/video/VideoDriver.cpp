#include "video/VideoDriver.h"

namespace lux
{
namespace video
{

static StrongRef<VideoDriver> g_VideoDriver;

void VideoDriver::Initialize(VideoDriver* driver)
{
	if(!driver)
		throw core::ErrorException("No video driver available");
	g_VideoDriver = driver;
}

VideoDriver* VideoDriver::Instance()
{
	return g_VideoDriver;
}

void VideoDriver::Destroy()
{
	g_VideoDriver.Reset();
}

}
}
