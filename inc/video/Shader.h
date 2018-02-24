#ifndef INCLUDED_SHADER_H
#define INCLUDED_SHADER_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/Attributes.h"
#include "video/videoExceptions.h"

namespace lux
{
namespace core
{
class ParamPackage;
}
namespace video
{
class Pass;
class Shader : public ReferenceCounted
{
public:
	virtual ~Shader() {}

	//! Make shader active
	virtual void Enable() = 0;
	//! Load a param into an active shader
	virtual void SetParam(u32 paramId, const void* data) = 0;
	//! Load all scene params into an active shader.
	virtual void LoadSceneParams(const Pass& pass) = 0;
	//! Called each time before the shader is rendererd.
	virtual void Render() = 0;
	//! Make shader inactive
	virtual void Disable() = 0;

	virtual const core::ParamPackage& GetParamPackage() const = 0;
	virtual u32 GetParamId(const core::String& name) const = 0;

	virtual size_t GetSceneParamCount() const = 0;
	virtual core::AttributePtr GetSceneParam(size_t id) const = 0;
};

} // namespace video
} // namespace lux

#endif
