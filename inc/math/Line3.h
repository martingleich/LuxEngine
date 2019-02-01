#ifndef INCLUDED_LUX_LINE3D_H
#define INCLUDED_LUX_LINE3D_H
#include "math/Vector3.h"

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

	Line3(T x1, T y1, T z1, T x2, T y2, T z2) :
		start(x1, y1, z1),
		end(x2, y2, z2)
	{
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

};

//! typdef for 3D Line with floating precision
typedef Line3<float> Line3F;

} // namespace math
} // namespace lux

#endif