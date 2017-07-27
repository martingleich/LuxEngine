#ifndef INCLUDED_LINE3D_H
#define INCLUDED_LINE3D_H
#include "math/vector3.h"

namespace lux
{
namespace math
{

//! Specifies a limited line in the third dimension
template <typename T>
class Line3
{
public:
	Vector3<T> start;    //! The startpoint of the line
	Vector3<T> end;        //! The endpoint of the line

	//! Defaultconstructor: A noexisting line from 0 to 0
	Line3() : start(0, 0, 0), end(0, 0, 0)
	{
	}
	//! Constructor from one point to another
	/**
	\param start The startpoint of the line
	\param end The endpoint of the line
	*/
	Line3(const Vector3<T>& start, const Vector3<T> end) : start(start), end(end)
	{
	}

	Line3(T x1, T y1, T z1, T x2, T y2, T z2) : start(x1, y1, z1), end(x2, y2, z2)
	{
	}

	Line3<T>& operator=(const Line3<T>& other)
	{
		start = other.start; end = other.end; return *this;
	}

	//! Equality of two lines
	/**
	Are the to lines identical, the direction is unimportat
	\param other The line to compare
	\return Are the lines equal
	*/
	bool operator==(const Line3<T>& other) const
	{
		return (start == other.start && end == other.end) || (start == other.end && end == other.start);
	}

	//! Unequality of two lines
	/**
	\param other The line to compare
	\return Are the lines unequal
	*/
	bool operator!=(const Line3<T>& other) const
	{
		return !(other == *this);
	}

	//! The length of the line
	/**
	\return The length of the line
	*/
	T GetLength() const
	{
		return start.GetDistanceTo(end);
	}

	//! The squared length of the line
	/**
	Much faster than GetLength()
	\return The squared length of the line
	*/
	T GetLengthSq() const
	{
		return start.GetDistanceToSq(end);
	}

	//! Retrieves the point in the middle of the line
	/**
	\return The middle point of the line
	*/
	Vector3<T> GetMiddle() const
	{
		return (start + end) / (T)(2);
	}

	//! Retrieves a point on the line
	/**
	Performs a interpolation between start and end with interpolationparam p
	\param p Specifies which point on the line 0 is the startpoint, 1 is the end point
	\return The point
	*/
	Vector3<T> GetPoint(float p) const
	{
		return start + p*(end - start);
	}

	//! The directionvector of the line from start to end, the vector has the length of the line
	/**
	\return The direction vector of the line
	*/
	Vector3<T> GetVector() const
	{
		return end - start;
	}

	//! Check if a point is on the line
	/**
	\param p The point to check, this point must be kolinear with the line
	\return Is the point between start and end
	*/
	bool IsPointOnLimited(const Vector3<T>& p) const
	{
		return p.IsBetweenPoints(start, end);
	}

	//! The nearest point on the line
	/**
	\param Point The point to compare with
	\return The nearest point to the param, which is on the line
	*/
	Vector3<T> GetClosestPoint(const Vector3<T>& Point) const
	{
		const Vector3<T> v(end - start);
		const Vector3<T> w(Point - start);

		const T d = w.Dot(v) / v.GetLengthSq();
		if(d < (T)0.0)
			return start;
		else if(d > (T)0)
			return end;

		v *= d;
		return start + v;
	}

	//! Check for intersection with a sphere
	/**
	\param s The center of the sphere
	\param r The radius of the sphere
	\param outLineSegment Here the linesegemnt of the first intersection is written
	\return Is there a intersection between the line and the sphere
	*/
	bool HitsSphere(const Vector3<T>& s,
		T r, float& outLineSegment) const
	{
		outLineSegment = (T)0;

		const Vector3<T> p = start - s;
		if(p.GetLengthSq() <= r*r)
			return true;

		const Vector3<T> u = GetVector();
		const T x = u.Dot(p);
		const T DirSq = u.GetLengthSq();

		const T d = x*x - DirSq * (p.GetLengthSq() - r);
		if(d < (T)0)
			return false;
		else if(IsZero(d))
			outLineSegment = -x * DirSq;
		else
			outLineSegment = (-x - sqrt(d)) * DirSq;

		if(outLineSegment < (T)0 || outLineSegment >(T)1)
			return false;
		else
			return true;
	}

	//! Retrieves the distance to another line
	/**
	\param other The other line
	\return The distance to the other line
	*/
	float GetDinstanceTo(const Line3& other) const
	{
		Vector3F u = end - start;
		Vector3F v = other.end - other.start;
		Vector3F w = start - other.start;
		float a = u.GetLengthSq();
		float c = v.GetLengthSq();
		float b = u.Dot(v);
		float d = u.Dot(w);
		float e = v.Dot(w);
		float D = a*c - b*b;

		if(IsZero(D)) {
			float tc = b > c ? d / b : e / c;

			return (w - v*tc).GetLength();
		} else {
			float sc = (b*e - c*d) / D;
			float tc = (a*e - b*d) / D;

			return (w + u*sc - v*tc).GetLength();
		}
	}
};

//! typdef for 3D Line with floating precision
typedef Line3<float> Line3F;

} // namespace math
} // namespace lux

#endif