#ifndef INCLUDED_LUX_VIDEO_EXCEPTIONS_
#define INCLUDED_LUX_VIDEO_EXCEPTIONS_
#include "core/lxException.h"

namespace lux
{
namespace video
{

struct ShaderCompileException : public core::Exception
{
	explicit ShaderCompileException(const char* msg = "shader compile error") :
		Exception(msg)
	{}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_VIDEO_EXCEPTIONS_