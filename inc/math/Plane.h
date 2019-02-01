#ifndef INCLUDED_LUX_PLANE3D_H
#define INCLUDED_LUX_PLANE3D_H
#include "math/Vector3.h"

namespace lux
{
namespace math
{

//! Object for planes in 3D
template <typename T>
class Plane
{
public:
	//! The normal of the plane, must have length one at all times.
	Vector3<T> normal;
	T d;

	//! default constructor the XZ-Plane
	Plane() :
		normal(0, 1, 0),
		d(0)
	{
	}
	//! Construct from Normal and d value
	Plane(const Vector3<T>& _Normal, const T d)
	{
		SetPlane(_Normal, d)
	}
	//! Construct from a point on the plane and the normal
	Plane(const Vector3<T>& Point, const Vector3<T>& _Normal)
	{
		SetPlane(Point, _Normal);
	}
	//! Constructor from three points on the plane
	Plane(const Vector3<T>& A, const Vector3<T>& B, const Vector3<T>& C)
	{
		SetPlane(A, B, C);
	}

	bool operator==(const Plane<T>& other) const { return d == other.d && normal == other.normal; }
	bool operator!= (const Plane<T>& other) const { return !(*this == other); }

	//! Construct a plane from normalvector and a memberpoint
	/**
	\param point A memberpoint of the plane
	\param _Normal The normalvector of the plane
	*/
	void SetPlane(const Vector3<T>& point, const Vector3<T>& _Normal)
	{
		normal = _Normal;
		d = -(point.Dot(normal));
	}

	//! Construct a plane from Normalvektor and d
	/**
	\param _Normal Normalvector of the plane
	\param d d value of the plane
	*/
	void SetPlane(const Vector3<T>& _Normal, T d)
	{
		normal = _Normal;
		d = d;
	}

	//! Construct a plane from 3 Points
	/**
	\param A First point on the plane
	\param B Second point on the plane
	\param C Third point on the plane
	*/
	void SetPlane(const Vector3<T>& A, const Vector3<T>& B, const Vector3<T>& C)
	{
		normal = ((B - A).Cross(C - A)).Normal();
		d = -(A.Dot(normal));
	}

	//! Normalize the plane
	/**
	After this operation the Normalvector of the plane has the length 1.
	All more complex plane operations require a normlized plane
	\return A reference to the current plane
	*/
	Plane<T>& Normalize()
	{
		const T invLength = 1 / normal.GetLength();
		normal *= invLength;
		d *= invLength;
		return *this;
	}

	//! Return any point which lies on the plane
	/**
	\return A point located on the plane
	*/
	Vector3<T> GetMemberPoint() const
	{
		return normal * -d;
	}
};

//! typedef for plane with float precision
typedef Plane<float> PlaneF;

} // namespace math
} // namespace lux

#endif