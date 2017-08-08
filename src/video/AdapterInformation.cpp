#include "video/DriverConfig.h"

namespace lux
{
namespace video
{

u32 Adapter::GetMaxMultisampleLevel(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat)
{
	auto levels = GenerateMultisampleLevels(mode, windowed, backBuffer, zsFormat);

	u32 max = 0;
	for(auto it = levels.First(); it != levels.End(); ++it) {
		if(*it > max)
			max = *it;
	}

	return max;
}

bool Adapter::GetMatchingMode(DisplayMode& outMode, const math::Dimension2U& minRes, bool windowed, u32 minRefresh)
{
	auto modes = GenerateDisplayModes(windowed);
	u32 bestError = std::numeric_limits<u32>::max();
	for(auto it = modes.First(); it != modes.End(); ++it) {
		if(it->width >= minRes.width && it->height >= minRes.height && it->refreshRate >= minRefresh) {
			u32 error = it->width*it->height - minRes.GetArea();
			if(error < bestError) {
				outMode = *it;
				bestError = error;
				if(bestError == 0)
					return true;
			}
		}
	}

	return (bestError != std::numeric_limits<u32>::max());
}

bool Adapter::GetMatchingBackbuffer(ColorFormat& outFormat, const DisplayMode& mode, bool windowed, bool use16Bit)
{
	auto formats = GenerateBackbufferFormats(mode, windowed);

	for(auto it = formats.First(); it != formats.End(); ++it) {
		if(use16Bit) {
			if(it->GetBitsPerPixel() != 16)
				continue;
		}

		outFormat = *it;
		return true;
	}

	return false;
}
bool Adapter::GetMatchingZStencil(ZStencilFormat& outFormat, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, u32 minDepth, u32 minStencil)
{
	auto formats = GenerateZStencilFormats(mode, windowed, backBuffer);

	for(auto it = formats.First(); it != formats.End(); ++it) {
		if(it->sBits < minStencil || it->zBits < minDepth)
			continue;

		outFormat = *it;
		return true;
	}

	return false;
}
bool Adapter::GetMatchingMultisample(u32& outLevel, u32& outQuality, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, u32 minSamples, u32 minQuality)
{
	auto levels = GenerateMultisampleLevels(mode, windowed, backBuffer, zsFormat);

	for(auto it = levels.First(); it != levels.End(); ++it) {
		if(*it < minSamples)
			continue;

		u32 numQualities = GetNumMultisampleQualities(mode, windowed, backBuffer, zsFormat, *it);
		if(numQualities == 0)
			continue;
		u32 maxQuality = numQualities-1;
		if(minQuality > maxQuality)
			continue;

		outLevel = minSamples;
		outQuality = minQuality;
		return true;
	}

	return false;
}

bool Adapter::GenerateConfig(
	video::DriverConfig& outConfig,
	const math::Dimension2U& minRes,
	bool windowed, bool vSync,
	bool backBuffer16Bit,
	u32 minDepth,
	u32 minStencil,
	int multiSample)
{
	outConfig.adapter = this;

	if(!GetMatchingMode(outConfig.display, minRes, windowed))
		return false;

	outConfig.windowed = windowed;
	outConfig.vSync = vSync;

	if(!GetMatchingBackbuffer(outConfig.backBufferFormat, outConfig.display, outConfig.windowed, backBuffer16Bit))
		return false;

	if(!GetMatchingZStencil(outConfig.zsFormat, outConfig.display, outConfig.windowed, outConfig.backBufferFormat, minDepth, minStencil))
		return false;

	multiSample = math::Clamp(multiSample, 0, 10);
	u32 maxMultisample = GetMaxMultisampleLevel(outConfig.display, outConfig.windowed, outConfig.backBufferFormat, outConfig.zsFormat);
	u32 realMultisample = math::Lerp<u32>(0, maxMultisample, multiSample*0.1f);
	if(!GetMatchingMultisample(outConfig.multiSamples, outConfig.multiQuality, outConfig.display, outConfig.windowed, outConfig.backBufferFormat, outConfig.zsFormat, realMultisample, 0))
		return false;

	return true;
}
}
}