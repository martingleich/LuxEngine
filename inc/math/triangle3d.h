#ifndef INCLUDED_TRIANGLE3D_H
#define INCLUDED_TRIANGLE3D_H
#include "math/aabbox3d.h"

namespace lux
{
namespace math
{

//! A triangle in the third dimension
template <typename type>
class triangle3d
{
public:
	vector3<type> A;    //!< The first point of the triangle
	vector3<type> B;    //!< The second point of the triangle
	vector3<type> C;    //!< The third point of the triangle

public:
	//! default-Constructor: All 0 triangle
	triangle3d()
	{
	}
	//! Constructor from 3 points
	triangle3d(const vector3<type>& a,
		const vector3<type>& b,
		const vector3<type>& c) : A(a), B(b), C(c)
	{
	}

	//! Equality
	bool operator==(const triangle3d<type>& other) const
	{
		return other.A == A && other.B == other.B && other.C == C;
	}

	//! Inequality
	bool operator!=(const triangle3d<type>& other) const
	{
		return other.A != A || other.B != B || other.C != C;
	}

	//! Set triangle points
	triangle3d<type>& Set(const vector3<type>& a, const vector3<type>& b, const vector3<type>& c)
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
	vector3<type> GetNormal() const
	{
		return (B - A).Cross(C - A);
	}

	//! The center of the triangle
	/**
	\return The center of the triangle
	*/
	vector3<type> GetCenter() const
	{
		return (A + B + C) / 3;
	}

	//! The plane of the triangle
	/**
	Plane is normalized
	\return The plane of the triangle
	*/
	plane3d<type> GetPlane() const
	{
		return plane3d<type>(A, B, C);
	}

	//! The area of the triangle
	/**
	\return The area of the triangle
	*/
	type GetArea() const
	{
		return (B - A).Cross(C - A).GetLength() / (type)(2);
	}

	type GetCircumference() const
	{
		return (B-A).GetLength() + (A-C).GetLength() + (C-B).GetLength();
	}

	//! Determinates if the triangle is totally inside a bounding box
	/**
	\param box The box test against
	\return Is the triangle totally inside the box
	*/
	bool IsTotalInsideBox(const aabbox3d<type>& box) const
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
	vector3<type> ClosestPointOnTriangle(const vector3<type>& point) const
	{
		const vector3<type> pointToEdgeAB = line3d<type>(A, B).GetClosestPoint(point);
		const vector3<type> pointToEdgeBC = line3d<type>(B, C).GetClosestPoint(point);
		const vector3<type> pointToEdgeAC = line3d<type>(A, C).GetDinstanceTo(point);

		const type toEdgeAB = pointToEdgeAB.GetDistanceToSq(point);
		const type toEdgeBC = pointToEdgeBC.GetDistanceToSq(point);
		const type toEdgeAC = pointToEdgeAC.GetDistanceToSq(point);

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
	bool IntersectWithLine(const line3d<type>& line, vector3<type>* out = nullptr) const
	{
		const vector3<type> AB = A - B;
		const vector3<type> BC = B - C;
		const vector3<type> Normal = AB.Cross(BC);

		plane3d<type> plane = plane3d<type>(A, Normal);
		type s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const vector3<type> point = line.GetPoint(s);

		vector3<type> subNormal = AB.Cross(Normal);
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
	bool IntersectWithLineBary(const line3d<type>& line, vector3<type>* out = nullptr) const
	{
		const vector3<type> a(C - A);
		const vector3<type> b(B - A);

		plane3d<type> plane = plane3d<type>(A, a.Cross(b));
		type s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const vector3<type> c(line.GetPoint(s) - A);

		type dotAA = a.Dot(a);
		type dotAB = a.Dot(b);
		type dotAC = a.Dot(c);
		type dotBB = b.Dot(b);
		type dotBC = b.Dot(c);

		type invDenom = 1 / (dotAA * dotBB - dotAB * dotAB);

		type u = (dotBB * dotAC - dotAB * dotBC) * invDenom;
		type v = (dotAA * dotBC - dotAB * dotAC) * invDenom;

		if(
			(u > -math::Constants<type>::rounding_error()) &&
			(v > -math::Constants<type>::rounding_error()) &&
			(u + v < 1 + math::Constants<type>::rounding_error())) {

			if(out)
				*out = line.GetPoint(s);
			return true;
		}

		return false;
	}

};

//! Typedef for triangles with float precision
typedef triangle3d<float> triangle3df;


}    // namespace math
}    // namespace lux

#endif