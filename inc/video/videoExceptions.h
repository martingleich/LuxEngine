#ifndef INCLUDED_LUX_VIDEO_EXCEPTIONS_H
#define INCLUDED_LUX_VIDEO_EXCEPTIONS_H
#include "core/lxException.h"

namespace lux
{
namespace video
{

struct UnhandledShaderCompileErrorException : public core::RuntimeException
{
	core::ExceptionSafeString What() const { return "UnhandledShaderCompileErrorException"; }
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_VIDEO_EXCEPTIONS_H
