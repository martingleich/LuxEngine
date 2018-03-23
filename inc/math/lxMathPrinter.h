#ifndef INCLUDED_LUX_MATH_PRINTER_H
#define INCLUDED_LUX_MATH_PRINTER_H
#include "core/StringConverter.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Matrix4.h"

namespace lux
{
namespace math
{
/*
template <typename T, typename T2>
core::BasicStringBuffer<T>& operator<<(core::BasicStringBuffer<T>& buffer, const vector3<T2>& v)
{
	T2 a[] = {v.x, v.y, v.z};
	return buffer.AppendList(a, 3);
}


template <typename T, typename T2>
core::BasicStringBuffer<T>& operator<<(core::BasicStringBuffer<T>& buffer, const vector2<T2>& v)
{
	T2 a[] = {v.x, v.y};
	return buffer.AppendList(a, 2);
}

template <typename T>
core::BasicStringBuffer<T>& operator<<(core::BasicStringBuffer<T>& buffer, const matrix4& m)
{
	return buffer.AppendList(m.DataRowMajor(), 16);
}
*/
}
}

#endif // #ifndef INCLUDED_LUX_MATH_PRINTER_H
