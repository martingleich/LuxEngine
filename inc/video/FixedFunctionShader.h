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
	FixedFunctionParameters(
		const core::Array<core::String>& _textures,
		const core::Array<TextureStageSettings>& _stages,
		bool _useVertexColors = false,
		bool _enableFogging = true,
		int _maxLightCount = 4) :
		textures(_textures),
		stages(_stages),
		useVertexColors(_useVertexColors),
		enableFogging(_enableFogging),
		maxLightCount(_maxLightCount)
	{
	}

	core::Array<core::String> textures;
	core::Array<TextureStageSettings> stages;
	bool useVertexColors;
	bool enableFogging;
	int maxLightCount;

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
