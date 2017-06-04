#ifndef INCLUDED_DEVICE_STATE_H
#define INCLUDED_DEVICE_STATE_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{
class TextureLayer;
class TextureStageSettings;
class AlphaBlendSettings;
class PipelineSettings;
class Shader;

class DeviceState
{
public:
	virtual ~DeviceState() {}

	virtual void EnablePipeline(const PipelineSettings& pipeline) = 0;

	virtual void EnableTextureLayer(u32 stage, const TextureLayer& layer) = 0;
	virtual void EnableTextureStage(u32 stage, const TextureStageSettings& settings) = 0;
	virtual void DisableTextureStage(u32 stage) = 0;

	virtual void EnableVertexData() = 0;
	virtual void DisableVertexData() = 0;

	virtual void EnableAlpha(const AlphaBlendSettings& settings) = 0;
	virtual void DisableAlpha() = 0;

	virtual void EnableShader(Shader* s) = 0;
	virtual void DisableCurShader() = 0;

	virtual void* GetLowLevelDevice() = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_DEVICE_STATE_H