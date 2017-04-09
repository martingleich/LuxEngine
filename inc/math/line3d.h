#ifndef INCLUDED_LINE3D_H
#define INCLUDED_LINE3D_H

#include "math/vector3.h"

namespace lux
{
namespace math
{

//! Specifies a limited line in the third dimension
template <typename type>
class line3d
{
public:
	vector3<type> start;    //! The startpoint of the line
	vector3<type> end;        //! The endpoint of the line

	//! Defaultconstructor: A noexisting line from 0 to 0
	line3d() : start(0, 0, 0), end(0, 0, 0)
	{
	}
	//! Constructor from one point to another
	/**
	\param start The startpoint of the line
	\param end The endpoint of the line
	*/
	line3d(const vector3<type>& start, const vector3<type> end) : start(start), end(end)
	{
	}

	line3d<type>& operator=(const line3d<type>& other)
	{
		start = other.start; end = other.end; return *this;
	}

	//! Equality of two lines
	/**
	Are the to lines identical, the direction is unimportat
	\param other The line to compare
	\return Are the lines equal
	*/
	bool operator==(const line3d<type>& other) const
	{
		return (start == other.start && end == other.end) || (start == other.end && end == other.start);
	}

	//! Unequality of two lines
	/**
	\param other The line to compare
	\return Are the lines unequal
	*/
	bool operator!=(const line3d<type>& other) const
	{
		return !(other == *this);
	}

	//! The length of the line
	/**
	\return The length of the line
	*/
	type GetLength() const
	{
		return start.GetDistanceTo(end);
	}

	//! The squared length of the line
	/**
	Much faster than GetLength()
	\return The squared length of the line
	*/
	type GetLengthSq() const
	{
		return start.GetDistanceToSq(end);
	}

	//! Retrieves the point in the middle of the line
	/**
	\return The middle point of the line
	*/
	vector3<type> GetMiddle() const
	{
		return (start + end) / (type)(2);
	}

	//! Retrieves a point on the line
	/**
	Performs a interpolation between start and end with interpolationparam p
	\param p Specifies which point on the line 0 is the startpoint, 1 is the end point
	\return The point
	*/
	vector3<type> GetPoint(float p) const
	{
		return start + p*(end - start);
	}

	//! The directionvector of the line from start to end, the vector has the length of the line
	/**
	\return The direction vector of the line
	*/
	vector3<type> GetVector() const
	{
		return end - start;
	}

	//! Check if a point is on the line
	/**
	\param p The point to check, this point must be kolinear with the line
	\return Is the point between start and end
	*/
	bool IsPointOnLimited(const vector3<type>& p) const
	{
		return p.IsBetweenPoints(start, end);
	}

	//! The nearest point on the line
	/**
	\param Point The point to compare with
	\return The nearest point to the param, which is on the line
	*/
	vector3<type> GetClosestPoint(const vector3<type>& Point) const
	{
		const vector3<type> v(end - start);
		const vector3<type> w(Point - start);

		const type d = w.Dot(v) / v.GetLengthSq();
		if(d < (type)0.0)
			return start;
		else if(d > (type)0)
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
	bool HitsSphere(const vector3<type>& s,
		type r, float& outLineSegment) const
	{
		outLineSegment = (type)0;

		const vector3<type> p = start - s;
		if(p.GetLengthSq() <= r*r)
			return true;

		const vector3<type> u = GetVector();
		const type x = u.Dot(p);
		const type DirSq = u.GetLengthSq();

		const type d = x*x - DirSq * (p.GetLengthSq() - r);
		if(d < (type)0)
			return false;
		else if(IsZero(d))
			outLineSegment = -x * DirSq;
		else
			outLineSegment = (-x - sqrt(d)) * DirSq;

		if(outLineSegment < (type)0 || outLineSegment >(type)1)
			return false;
		else
			return true;
	}

	//! Retrieves the distance to another line
	/**
	\param other The other line
	\return The distance to the other line
	*/
	float GetDinstanceTo(const line3d& other) const
	{
		vector3f u = end - start;
		vector3f v = other.end - other.start;
		vector3f w = start - other.start;
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
typedef line3d<float> line3df;

}    }    
#endif