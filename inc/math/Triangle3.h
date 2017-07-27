#ifndef INCLUDED_TRIANGLE3D_H
#define INCLUDED_TRIANGLE3D_H
#include "math/AABBox.h"

namespace lux
{
namespace math
{

//! A triangle in the third dimension
template <typename T>
class Triangle3
{
public:
	Vector3<T> A;    //!< The first point of the triangle
	Vector3<T> B;    //!< The second point of the triangle
	Vector3<T> C;    //!< The third point of the triangle

public:
	//! default-Constructor: All 0 triangle
	Triangle3()
	{
	}
	//! Constructor from 3 points
	Triangle3(const Vector3<T>& a,
		const Vector3<T>& b,
		const Vector3<T>& c) : A(a), B(b), C(c)
	{
	}

	//! Equality
	bool operator==(const Triangle3<T>& other) const
	{
		return other.A == A && other.B == other.B && other.C == C;
	}

	//! Inequality
	bool operator!=(const Triangle3<T>& other) const
	{
		return other.A != A || other.B != B || other.C != C;
	}

	//! Set triangle points
	Triangle3<T>& Set(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& c)
	{
		A = a;
		B = b;
		C = c;

		return *this;
	}

	//! The normal of the triangle
	/**
	Computed normal is not normalized
	\return The normal of the triangle
	*/
	Vector3<T> GetNormal() const
	{
		return (B - A).Cross(C - A);
	}

	//! The center of the triangle
	/**
	\return The center of the triangle
	*/
	Vector3<T> GetCenter() const
	{
		return (A + B + C) / 3;
	}

	//! The plane of the triangle
	/**
	Plane is normalized
	\return The plane of the triangle
	*/
	Plane<T> GetPlane() const
	{
		return Plane<T>(A, B, C);
	}

	//! The area of the triangle
	/**
	\return The area of the triangle
	*/
	T GetArea() const
	{
		return (B - A).Cross(C - A).GetLength() / (T)(2);
	}

	T GetCircumference() const
	{
		return (B-A).GetLength() + (A-C).GetLength() + (C-B).GetLength();
	}

	//! Determinates if the triangle is totally inside a bounding box
	/**
	\param box The box test against
	\return Is the triangle totally inside the box
	*/
	bool IsTotalInsideBox(const AABBox<T>& box) const
	{
		return (box.IsPointInside(A) &&
			box.IsPointInside(B) &&
			box.IsPointInside(C));
	}

	//! Get the closest point on a triangle to another point
	/**
	\param point Point which must be on the same plane as the triangle
	\return The nearest point on the triangle
	*/
	Vector3<T> ClosestPointOnTriangle(const Vector3<T>& point) const
	{
		const Vector3<T> pointToEdgeAB = Line3<T>(A, B).GetClosestPoint(point);
		const Vector3<T> pointToEdgeBC = Line3<T>(B, C).GetClosestPoint(point);
		const Vector3<T> pointToEdgeAC = Line3<T>(A, C).GetDinstanceTo(point);

		const T toEdgeAB = pointToEdgeAB.GetDistanceToSq(point);
		const T toEdgeBC = pointToEdgeBC.GetDistanceToSq(point);
		const T toEdgeAC = pointToEdgeAC.GetDistanceToSq(point);

		if(toEdgeAB < toEdgeBC)
			return toEdgeAB < toEdgeAC ? pointToEdgeAB : pointToEdgeAC;

		return toEdgeBC < toEdgeAC ? pointToEdgeBC : pointToEdgeAC;
	}

	//! Check if a line intersect with the triangle(include border)
	/**
	\param line Line to check
	\param out If != NULL the coords of the intersection are written there
	\return True if there is a intersection otherwise false
	*/
	bool IntersectWithLine(const Line3<T>& line, Vector3<T>* out = nullptr) const
	{
		const Vector3<T> AB = A - B;
		const Vector3<T> BC = B - C;
		const Vector3<T> Normal = AB.Cross(BC);

		Plane<T> plane = Plane<T>(A, Normal);
		T s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const Vector3<T> point = line.GetPoint(s);

		Vector3<T> subNormal = AB.Cross(Normal);
		if(point.Dot(subNormal) < A.Dot(subNormal))
			return false;

		subNormal = BC.Cross(Normal);
		if(point.Dot(subNormal) < B.Dot(subNormal))
			return false;

		subNormal = (A - C).Cross(Normal);
		if(point.Dot(subNormal) < A.Dot(subNormal))
			return false;

		if(out) *out = point;
		return true;
	}

	//! Check if a line intersect with the triangle(including border)
	/**
	Uses a barycentric approach, is faster but less exact
	\param line Line to check
	\param out If != NULL the coords of the intersection are written there
	\return True if there is a intersection otherwise false
	*/
	bool IntersectWithLineBary(const Line3<T>& line, Vector3<T>* out = nullptr) const
	{
		const Vector3<T> a(C - A);
		const Vector3<T> b(B - A);

		Plane<T> plane = Plane<T>(A, a.Cross(b));
		T s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const Vector3<T> c(line.GetPoint(s) - A);

		T dotAA = a.Dot(a);
		T dotAB = a.Dot(b);
		T dotAC = a.Dot(c);
		T dotBB = b.Dot(b);
		T dotBC = b.Dot(c);

		T invDenom = 1 / (dotAA * dotBB - dotAB * dotAB);

		T u = (dotBB * dotAC - dotAB * dotBC) * invDenom;
		T v = (dotAA * dotBC - dotAB * dotAC) * invDenom;

		if(
			(u > -math::Constants<T>::rounding_error()) &&
			(v > -math::Constants<T>::rounding_error()) &&
			(u + v < 1 + math::Constants<T>::rounding_error())) {

			if(out)
				*out = line.GetPoint(s);
			return true;
		}

		return false;
	}

};

//! Typedef for triangles with float precision
typedef Triangle3<float> Triangle3F;

} // namespace math
} // namespace lux

#endif