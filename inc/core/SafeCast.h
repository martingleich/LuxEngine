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
inline bool CheckedCast(FromT from, ToT& to)
{
	static_assert(std::is_integral<FromT>::value && std::is_integral<ToT>::value, "Can only cast intergral types");
	ifconst(std::is_unsigned<ToT>::value == std::is_unsigned<FromT>::value)
	{
		using BiggerT = core::Choose<sizeof(ToT) < sizeof(FromT), FromT, ToT>::type;
		if((BiggerT)from > (BiggerT)std::numeric_limits<ToT>::max())
			return false;
		if((BiggerT)from < (BiggerT)std::numeric_limits<ToT>::min())
			return false;
	}
	ifconst(std::is_unsigned<ToT>::value && std::is_signed<FromT>::value)
	{
		// signed to unsigend
		if(from < 0)
			return false;
		using BiggerT = core::Choose<sizeof(ToT) < sizeof(FromT), std::make_unsigned<FromT>::type, ToT>::type;
		if((BiggerT)from > (BiggerT)std::numeric_limits<ToT>::max())
			return false;
	}
	ifconst(std::is_signed<ToT>::value && std::is_unsigned<FromT>::value) {

		// unsigned to sigend
		using BiggerT = core::Choose<sizeof(ToT) < sizeof(FromT), std::make_signed<FromT>::type, ToT>::type;
		if((BiggerT)from > (BiggerT)std::numeric_limits<ToT>::max())
			return false;
	}
	to = (ToT)from;
	return true;
}

template <typename ToT, typename FromT>
inline ToT SafeCast(FromT from)
{
	ToT out;
	if(!CheckedCast(from, out))
		throw core::Exception("Overflow");
	return out;
}

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SAFE_CAST_H

