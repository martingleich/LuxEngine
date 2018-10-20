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
		bool _useVertexColors = false) :
		textures(_textures),
		stages(_stages),
		useVertexColors(_useVertexColors)
	{
	}

	core::Array<core::String> textures;
	core::Array<TextureStageSettings> stages;
	bool useVertexColors;

	bool operator==(const FixedFunctionParameters& other) const
	{
		return useVertexColors == other.useVertexColors && 
			textures == other.textures &&
			stages == other.stages;
	}
	
	bool operator!=(const  FixedFunctionParameters& other) const
	{
		return !(*this == other);
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
