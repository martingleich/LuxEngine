#ifndef INCLUDED_LUX_SHADER_H
#define INCLUDED_LUX_SHADER_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/Attributes.h"
#include "video/videoExceptions.h"
#include "video/VideoEnums.h"

namespace lux
{
namespace core
{
class ParamPackage;
}
namespace video
{

enum class EShaderCompileMessageLevel
{
	Error,
	Warning,
	Info
};

class ShaderCompileMessage
{
public:
	ShaderCompileMessage() {}
	ShaderCompileMessage(EShaderType _shaderType, EShaderCompileMessageLevel _level, core::StringView _text, int _line = -1) :
		shaderType(_shaderType),
		level(_level),
		text(_text),
		line(_line)
	{
	}
	EShaderType shaderType;
	EShaderCompileMessageLevel level;
	core::String text;
	int line;

	core::String ToString() const
	{
		const char* type;
		if(shaderType == EShaderType::Vertex)
			type = "vertexshader";
		else if(shaderType == EShaderType::Pixel)
			type = "pixelshader";
		else
			type = nullptr;
		if(type == nullptr || line < 0)
			return text;
		else
			return core::StringConverter::Format("{}: line {}: {}", type, line, text);
	}
};

class Pass;
class Shader : public ReferenceCounted
{
public:
	virtual ~Shader() {}

	//! Make shader active
	virtual void Enable() = 0;
	//! Load a param into an active shader
	virtual void SetParam(int paramId, const void* data) = 0;
	//! Load all scene params into an active shader.
	virtual void LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass) = 0;
	//! Called each time before the shader is rendererd.
	virtual void Render() = 0;
	//! Make shader inactive
	virtual void Disable() = 0;

	virtual const core::ParamPackage& GetParamPackage() const = 0;
	int GetParamId(core::StringView name) const
	{
		return GetParamPackage().GetParamIdByName(name);
	}
};

} // namespace video
} // namespace lux

#endif
