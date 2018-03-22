#include "video/DriverConfig.h"

namespace lux
{
namespace video
{

int Adapter::GetMaxMultisampleLevel(const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat)
{
	auto levels = GenerateMultisampleLevels(mode, windowed, backBuffer, zsFormat);

	int max = 0;
	for(auto it = levels.First(); it != levels.End(); ++it) {
		if(*it > max)
			max = *it;
	}

	return max;
}

bool Adapter::GetMatchingMode(DisplayMode& outMode, const math::Dimension2I& minRes, bool windowed, int minRefresh)
{
	auto modes = GenerateDisplayModes(windowed);
	int bestError = INT_MAX;
	for(auto it = modes.First(); it != modes.End(); ++it) {
		if(it->width >= minRes.width && it->height >= minRes.height && it->refreshRate >= minRefresh) {
			int error = it->width*it->height - minRes.GetArea();
			if(error < bestError) {
				outMode = *it;
				bestError = error;
				if(bestError == 0)
					return true;
			}
		}
	}

	return (bestError != INT_MAX);
}

bool Adapter::GetMatchingBackbuffer(ColorFormat& outFormat, const DisplayMode& mode, bool windowed, int minBits)
{
	auto formats = GenerateBackbufferFormats(mode, windowed);

	for(auto it = formats.First(); it != formats.End(); ++it) {
		if(it->GetBitsPerPixel() > minBits) {
			outFormat = *it;
			return true;
		}
	}

	return false;
}

bool Adapter::GetMatchingZStencil(ZStencilFormat& outFormat, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, int minDepth, int minStencil)
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

bool Adapter::GetMatchingMultisample(int& outLevel, int& outQuality, const DisplayMode& mode, bool windowed, ColorFormat backBuffer, ZStencilFormat zsFormat, int minSamples, int minQuality)
{
	auto levels = GenerateMultisampleLevels(mode, windowed, backBuffer, zsFormat);

	for(auto it = levels.First(); it != levels.End(); ++it) {
		if(*it < minSamples)
			continue;

		int numQualities = GetNumMultisampleQualities(mode, windowed, backBuffer, zsFormat, *it);
		if(numQualities == 0)
			continue;
		int maxQuality = numQualities - 1;
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
	const math::Dimension2I& minRes,
	bool windowed, bool vSync,
	int minDepth,
	int minStencil,
	int multiSample)
{
	outConfig.adapter = this;

	if(!GetMatchingMode(outConfig.display, minRes, windowed))
		return false;

	outConfig.windowed = windowed;
	outConfig.vSync = vSync;

	if(!GetMatchingBackbuffer(outConfig.backBufferFormat, outConfig.display, outConfig.windowed, 24))
		return false;

	if(!GetMatchingZStencil(outConfig.zsFormat, outConfig.display, outConfig.windowed, outConfig.backBufferFormat, minDepth, minStencil))
		return false;

	multiSample = math::Clamp(multiSample, 0, 10);
	auto maxMultisample = GetMaxMultisampleLevel(outConfig.display, outConfig.windowed, outConfig.backBufferFormat, outConfig.zsFormat);
	auto realMultisample = math::Lerp<int>(0, maxMultisample, multiSample*0.1f);
	if(!GetMatchingMultisample(outConfig.multiSamples, outConfig.multiQuality, outConfig.display, outConfig.windowed, outConfig.backBufferFormat, outConfig.zsFormat, realMultisample, 0))
		return false;

	return true;
}
}
}