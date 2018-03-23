#ifndef INCLUDED_LUX_SAFE_CAST_H
#define INCLUDED_LUX_SAFE_CAST_H
#include <limits>
#include "core/lxException.h"
#include "core/HelperTemplates.h"

namespace lux
{
namespace core
{

template <typename ToT, typename FromT>
inline ToT SafeCast(FromT from)
{
	using BiggerT = core::Choose<(sizeof(ToT) > sizeof(FromT)), ToT, FromT>::type;
	if((BiggerT)from > (BiggerT)std::numeric_limits<ToT>::max())
		throw core::Exception("Overflow");
	if((BiggerT)from < (BiggerT)std::numeric_limits<ToT>::min())
		throw core::Exception("Overflow");
	return static_cast<ToT>(from);
}

template <typename ToT, typename FromT>
inline ToT SaturateCast(FromT from)
{
	if(from > std::numeric_limits<ToT>::max())
		return std::numeric_limits<ToT>::max();
	if(from < std::numeric_limits<ToT>::min())
		return std::numeric_limits<ToT>::min();
	return static_cast<ToT>(from);
}

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SAFE_CAST_H

