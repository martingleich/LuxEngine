#ifndef INCLUDED_DEVICE_STATE_H
#define INCLUDED_DEVICE_STATE_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{
class TextureLayer;
class AlphaBlendSettings;
class Shader;

class DeviceState
{
public:
	virtual ~DeviceState() {}

	virtual void EnableTextureLayer(u32 stage, const TextureLayer& layer) = 0;
	virtual void EnableAlpha(const AlphaBlendSettings& settings) = 0;
	virtual void EnableShader(Shader* s) = 0;
	virtual void DisableCurShader() = 0;

	virtual void* GetLowLevelDevice() = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_DEVICE_STATE_H