#ifndef INCLUDED_PLANE3D_H
#define INCLUDED_PLANE3D_H
#include "math/Line3.h"

namespace lux
{
namespace math
{

//! Object for planes in 3D
template <typename type>
class Plane
{
public:
	//! Classifies a intersection with a plane
	enum class ERelation
	{
		Front = 0,       //!< The object is before the plane    
		Back,            //!< The object is behind the plane
		Plane,           //!< The object is on the plane
		Clipped          //!< The object is cut by the plane
	};

public:
	Vector3<type> normal;
	type d;

	//! default constructor the XZ-Plane
	Plane() : normal(0, 1, 0), d(0)
	{
	}
	//! Construct from Normal and d value
	Plane(const Vector3<type>& _Normal, const type d) : normal(_Normal), d(d)
	{
	}
	//! Construct from a point on the plane and the normal
	Plane(const Vector3<type>& Point, const Vector3<type>& _Normal) : normal(_Normal)
	{
		RecalculateD(Point);
	}
	//! Constructor from three points on the plane
	Plane(const Vector3<type>& A, const Vector3<type>& B, const Vector3<type>& C)
	{
		SetPlane(A, B, C);
	}

	//! Copyconstuctor
	Plane(const Plane& other) :
		normal(other.normal),
		d(other.d)
	{
	}

	//! Assignment
	Plane<type> operator= (const Plane<type> other)
	{
		normal = other.normal; d = other.d; return *this;
	}

	//! Equality
	inline bool operator== (const Plane<type>& other) const
	{
		return Equal(d, other.d) && normal == other.normal;
	}

	//! Unequality
	inline bool operator!= (const Plane<type>& other) const
	{
		return !(*this == other);
	}

	//! Construct a plane from normalvector and a memberpoint
	/**
	\param point A memberpoint of the plane
	\param _Normal The normalvector of the plane
	*/
	void SetPlane(const Vector3<type>& point, const Vector3<type>& _Normal)
	{
		normal = _Normal;
		RecalculateD(point);
	}

	//! Construct a plane from Normalvektor and d
	/**
	\param _Normal Normalvector of the plane
	\param d d value of the plane
	*/
	void SetPlane(const Vector3<type>& _Normal, type d)
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
	void SetPlane(const Vector3<type>& A, const Vector3<type>& B, const Vector3<type>& C)
	{
		normal = (B - A).Cross(C - A);
		normal.Normalize_s();
		RecalculateD(A);
	}

	//! Rebuild the plane for a new memberpoint
	/**
	\param point New Member point to apply
	*/
	void RecalculateD(const Vector3<type>& point)
	{
		d = -point.Dot(normal);
	}

	//! Normalize the plane
	/**
	After this operation the Normalvector of the plane has the length 1
	\return A reference to the current plane
	*/
	Plane<type>& Normalize()
	{
		const double dInvLength = 1.0 / normal.GetLength();
		normal *= dInvLength;
		d *= dInvLength;
		return *this;
	}

	//! Return any point which lies on the plane
	/**
	\return A point located on the plane
	*/
	Vector3<type> GetMemberPoint() const
	{
		return normal * -d;
	}

	//! Compute the minimal distance to a point
	/**
	\param p Point to which the distance should computed
	\return The distance to the point
	*/
	type GetDistanceTo(const Vector3<type>& p) const
	{
		type out = p.Dot(normal) + d;
		float lsq = normal.GetLengthSq();
		if(lsq != 1.0f)
			out /= sqrt(lsq);
		return out;
	}

	//! Check for an intersection with a line
	/**
	\param line Line to check the intersection
	\param out If != NULL, the linesegment intersected is written there
	\return True, if an intersection occured
	*/
	bool IntersectWithLine(const Line3<type>& line,
		float* out = nullptr) const
	{
		const float denominator = normal.Dot(line.GetVector());

		if(IsZero(denominator)) {
			// Line and plane are parallel
			if(IsZero(normal.Dot(line.start) + d)) {
				// Line lies on the plane
				if(out) *out = 0.0f;
				return true;
			} else {
				return false;
			}
		}

		const float segment = -(normal.Dot(line.start) + d) / denominator;

		if(segment < 0.0f || segment > 1.0f)
			return false;

		if(out) *out = segment;
		return true;
	}

	//! Check for an intersection with a line, not computing the exact intesection coords
	/**
	\param line Line to check the intersection
	\return True, if an intersection occured, False otherwise
	*/
	bool IntesectWithLineFast(const Line3<type>& line) const
	{
		const float d1 = GetDistanceTo(line.start);
		const float d2 = GetDistanceTo(line.end);

		// Both point are on diffrent sides of the plane, so there must be an intersection
		if(d1 <= 0.0f && d2 >= 0.0f) return true;
		if(d1 >= 0.0f && d2 <= 0.0f) return true;

		return false;
	}
};

//! typedef for plane with float precision
typedef Plane<float> PlaneF;

} // namespace math
} // namespace lux

#endif