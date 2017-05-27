#ifndef INCLUDED_SHADER_H
#define INCLUDED_SHADER_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"

namespace lux
{
namespace core
{
class ParamPackage;
}
namespace video
{
class RenderSettings;
struct ShaderCompileException : public core::Exception
{
	explicit ShaderCompileException(const char* msg = "shader compile error") :
		Exception(msg)
	{}
};

class Shader : public ReferenceCounted
{
public:
	virtual ~Shader() {}

	virtual void Enable() = 0;
	virtual void LoadSettings(const RenderSettings& settings) = 0;
	virtual void Disable() = 0;

	//! Initializes the shader from code
	/**
	\throws ShaderCompileException The shader couldn't be compiled, error because of code.
	*/
	virtual void Init(
		const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
		const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile,
		core::array<string>* errorList) = 0;

	virtual const core::ParamPackage& GetParamPackage() const = 0;

	virtual size_t GetSceneParamCount() const = 0;
	virtual u32 GetSceneParam(size_t id) const = 0;
};

} // namespace video
} // namespace lux

#endif
