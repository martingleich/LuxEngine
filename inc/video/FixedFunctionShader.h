#ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
#define INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
#include "video/Shader.h"
#include "video/TextureStageSettings.h"

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

class FixedFunctionShader : public Shader
{
public:
	FixedFunctionShader(const FixedFunctionParameters& params) :
		m_TextureStages(params.stages),
		m_UseVertexColors(params.useVertexColors)
	{
		for(auto& s : params.textures)
			m_ParamPackage.AddParam(s, TextureLayer());
	}

	virtual int GetTextureStageCount() const
	{
		return m_TextureStages.Size();
	}
	virtual const TextureStageSettings& GetTextureStage(int id)
	{
		return m_TextureStages[id];
	}

	const core::ParamPackage& GetParamPackage() const
	{
		return m_ParamPackage;
	}

protected:
	core::Array<TextureStageSettings> m_TextureStages;
	core::ParamPackage m_ParamPackage;
	bool m_UseVertexColors;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_H
