#ifndef INCLUDED_LUX_PLANE3D_H
#define INCLUDED_LUX_PLANE3D_H
#include "math/Line3.h"

namespace lux
{
namespace math
{

//! Object for planes in 3D
template <typename T>
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
	Vector3<T> normal;
	T d;

	//! default constructor the XZ-Plane
	Plane() : normal(0, 1, 0), d(0)
	{
	}
	//! Construct from Normal and d value
	Plane(const Vector3<T>& _Normal, const T d) : normal(_Normal), d(d)
	{
	}
	//! Construct from a point on the plane and the normal
	Plane(const Vector3<T>& Point, const Vector3<T>& _Normal) : normal(_Normal)
	{
		RecalculateD(Point);
	}
	//! Constructor from three points on the plane
	Plane(const Vector3<T>& A, const Vector3<T>& B, const Vector3<T>& C)
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
	Plane<T> operator= (const Plane<T> other)
	{
		normal = other.normal; d = other.d; return *this;
	}

	//! Equality
	inline bool operator== (const Plane<T>& other) const
	{
		return Equal(d, other.d) && normal == other.normal;
	}

	//! Unequality
	inline bool operator!= (const Plane<T>& other) const
	{
		return !(*this == other);
	}

	//! Construct a plane from normalvector and a memberpoint
	/**
	\param point A memberpoint of the plane
	\param _Normal The normalvector of the plane
	*/
	void SetPlane(const Vector3<T>& point, const Vector3<T>& _Normal)
	{
		normal = _Normal;
		RecalculateD(point);
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
		RecalculateD(A);
	}

	//! Rebuild the plane for a new memberpoint
	/**
	\param point New Member point to apply
	*/
	void RecalculateD(const Vector3<T>& point)
	{
		d = -point.Dot(normal);
	}

	//! Normalize the plane
	/**
	After this operation the Normalvector of the plane has the length 1.
	All more complex line operation require a normlized plane
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

	//! Compute the minimal distance to a point
	/**
	\param p Point to which the distance should computed
	\return The distance to the point
	*/
	T GetDistanceTo(const Vector3<T>& p) const
	{
		T out = p.Dot(normal) + d;
		return out;
	}

	//! Check for an intersection with a line
	/**
	\param line Line to check the intersection
	\param out If != NULL, the linesegment intersected is written there
	\return True, if an intersection occured
	*/
	bool IntersectWithLine(const Line3<T>& line,
		float* out = nullptr,
		bool isInfinite = false) const
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

		if(!isInfinite) {
			if(segment < 0.0f || segment > 1.0f)
				return false;
		}

		if(out)
			*out = segment;
		return true;
	}

	//! Check for an intersection with a line, not computing the exact intesection coords
	/**
	\param line Line to check the intersection
	\return True, if an intersection occured, False otherwise
	*/
	bool IntersectWithLineFast(const Line3<T>& line) const
	{
		const float d1 = GetDistanceTo(line.start);
		const float d2 = GetDistanceTo(line.end);

		// Both point are on diffrent sides of the plane, so there must be an intersection
		if(d1 <= 0.0f && d2 >= 0.0f) return true;
		if(d1 >= 0.0f && d2 <= 0.0f) return true;

		return false;
	}

	//! Classify point relation to plane
	ERelation ClassifyPoint(const math::Vector3<T>& point) const
	{
		auto x = normal.Dot(point) + d;
		if(x < -math::Constants<T>::rounding_error())
			return ERelation::Back;

		if(x > math::Constants<T>::rounding_error())
			return ERelation::Front;
		return ERelation::Plane;
	}

	ERelation ClassifyBox(const math::Vector3<T>& boxMin, const math::Vector3<T>& boxMax) const
	{
		auto nearPoint = boxMax;
		auto farPoint = boxMin;
		if(normal.x > 0) {
			nearPoint.x = boxMin.x;
			farPoint.x = boxMax.x;
		}
		if(normal.y > 0) {
			nearPoint.y = boxMin.y;
			farPoint.y = boxMax.y;
		}
		if(normal.z > 0) {
			nearPoint.z = boxMin.z;
			farPoint.z = boxMax.z;
		}
		if(normal.Dot(nearPoint) + d > 0)
			return ERelation::Front;
		if(normal.Dot(farPoint) + d > 0)
			return ERelation::Clipped;
		return ERelation::Back;
	}

	bool IntersectWithPlane(
		const math::Plane<T>& other,
		math::Line3<T>& outLine) const
	{
		const T fn01 = normal.Dot(other.normal);
		const T det = 1 - fn01*fn01;

		if(IsZero(det))
			return false;

		const T invdet = 1 / det;
		const T fc0 = (-d + fn01*other.d) * invdet;
		const T fc1 = (-other.d + fn01*d) * invdet;

		outLine.start = normal*fc0 + other.normal*fc1;
		outLine.end = outLine.start + normal.Cross(other.normal);
		return true;
	}

	bool IntersectWithPlanes(
		const math::Plane<T>& o1,
		const math::Plane<T>& o2,
		math::Vector3<T>& outPoint) const
	{
		math::Line3<T> line;
		if(!this->IntersectWithPlane(o1, line))
			return false;

		float seg;
		if(!o2.IntersectWithLine(line, &seg, true))
			return false;

		outPoint = line.GetPoint(seg);
		return true;
	}
};

//! typedef for plane with float precision
typedef Plane<float> PlaneF;

} // namespace math
} // namespace lux

#endif