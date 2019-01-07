#ifndef INCLUDED_LUX_FOG_DATA_H
#define INCLUDED_LUX_FOG_DATA_H
#include "video/Color.h"

namespace lux
{
namespace video
{

enum class EFogType
{
	Linear,
	Exp,
	ExpSq,
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FOG_DATA_H