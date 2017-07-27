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
	Vector3<type> A;    //!< The first point of the triangle
	Vector3<type> B;    //!< The second point of the triangle
	Vector3<type> C;    //!< The third point of the triangle

public:
	//! default-Constructor: All 0 triangle
	triangle3d()
	{
	}
	//! Constructor from 3 points
	triangle3d(const Vector3<type>& a,
		const Vector3<type>& b,
		const Vector3<type>& c) : A(a), B(b), C(c)
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
	triangle3d<type>& Set(const Vector3<type>& a, const Vector3<type>& b, const Vector3<type>& c)
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
	Vector3<type> GetNormal() const
	{
		return (B - A).Cross(C - A);
	}

	//! The center of the triangle
	/**
	\return The center of the triangle
	*/
	Vector3<type> GetCenter() const
	{
		return (A + B + C) / 3;
	}

	//! The plane of the triangle
	/**
	Plane is normalized
	\return The plane of the triangle
	*/
	Plane<type> GetPlane() const
	{
		return Plane<type>(A, B, C);
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
	bool IsTotalInsideBox(const AABBox<type>& box) const
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
	Vector3<type> ClosestPointOnTriangle(const Vector3<type>& point) const
	{
		const Vector3<type> pointToEdgeAB = Line3<type>(A, B).GetClosestPoint(point);
		const Vector3<type> pointToEdgeBC = Line3<type>(B, C).GetClosestPoint(point);
		const Vector3<type> pointToEdgeAC = Line3<type>(A, C).GetDinstanceTo(point);

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
	bool IntersectWithLine(const Line3<type>& line, Vector3<type>* out = nullptr) const
	{
		const Vector3<type> AB = A - B;
		const Vector3<type> BC = B - C;
		const Vector3<type> Normal = AB.Cross(BC);

		Plane<type> plane = Plane<type>(A, Normal);
		type s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const Vector3<type> point = line.GetPoint(s);

		Vector3<type> subNormal = AB.Cross(Normal);
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
	bool IntersectWithLineBary(const Line3<type>& line, Vector3<type>* out = nullptr) const
	{
		const Vector3<type> a(C - A);
		const Vector3<type> b(B - A);

		Plane<type> plane = Plane<type>(A, a.Cross(b));
		type s;
		if(!plane.IntersectWithLine(line, &s))
			return false;

		const Vector3<type> c(line.GetPoint(s) - A);

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


}    

}    


#endif