#ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
#define INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
#include "video/Shader.h"
#include "video/TextureStageSettings.h"
#include "video/TextureLayer.h"

namespace lux
{
namespace video
{

struct FixedFunctionParameters
{
	static FixedFunctionParameters VertexColorOnly()
	{
		FixedFunctionParameters out;
		out.enableFogging = false;
		out.maxLightCount = 0;
		out.useVertexColors = true;
		return out;
	}
	static FixedFunctionParameters Unlit(
		const core::Array<core::String>& textures,
		const core::Array<TextureStageSettings>& stages,
		bool useVertexColors=false)
	{
		FixedFunctionParameters out;
		out.textures = textures;
		out.stages = stages;
		out.enableFogging = false;
		out.maxLightCount = 0;
		out.useVertexColors = useVertexColors;
		return out;
	}
	core::Array<core::String> textures;
	core::Array<TextureStageSettings> stages;
	bool useVertexColors = false;
	bool enableFogging = false;
	int maxLightCount = 0;

	bool operator==(const FixedFunctionParameters& other) const
	{
		return
			textures == other.textures &&
			stages == other.stages &&
			useVertexColors == other.useVertexColors &&
			enableFogging == other.enableFogging &&
			maxLightCount == other.maxLightCount;
	}

	bool operator!=(const  FixedFunctionParameters& other) const
	{
		return !(*this == other);
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
