#ifndef INCLUDED_AABBOX3D_H
#define INCLUDED_AABBOX3D_H
#include "plane3d.h"

namespace lux
{
namespace math
{

//! Object for axis-aligned Bounding-Boxes
template <typename type>
class aabbox3d
{
public:
	//! The edge which is nearer to the (-,-,-)-Spacecube
	vector3<type> minCorner;
	//! The edge which is nearer to the (+,+,+)-Spacecube
	vector3<type> maxCorner;

	//! An empty box located at the origin
	static const aabbox3d<type> EMPTY;

	//! default-Constructor
	aabbox3d() : minCorner(-1, -1, -1), maxCorner(1, 1, 1)
	{
	}
	//! from two edges
	aabbox3d(const vector3<type>& min, const vector3<type>& max) : minCorner(min), maxCorner(max)
	{
	}
	//! from a single point
	aabbox3d(const vector3<type>& start) : minCorner(start), maxCorner(start)
	{
	}
	//! from two edges in single Coords
	aabbox3d(type minX, type minY, type minZ, type maxX, type maxY, type maxZ) : minCorner(minX, minY, minZ), maxCorner(maxX, maxY, maxZ)
	{
	}

	inline bool operator== (const aabbox3d<type>& other) const
	{
		return (minCorner == other.minCorner && maxCorner == other.maxCorner);
	}
	inline bool operator!= (const aabbox3d<type>& other) const
	{
		return !(*this == other);
	}
	aabbox3d<type>& operator= (const aabbox3d<type>& other)
	{
		minCorner = other.minCorner; maxCorner = other.maxCorner; return *this;
	}

	//! Add another point to the box
	aabbox3d<type>& operator+= (const vector3<type>& v)
	{
		AddPoint(v); return *this;
	}
	//! Add another box to the box
	aabbox3d<type>& operator+= (const aabbox3d<type>& b)
	{
		AddBox(b); return *this;
	}

	//! Set the box to a single point
	void Set(type X, type Y, type Z)
	{
		minCorner.Set(X, Y, Z);
		maxCorner = minCorner;
	}

	//! Set the box to a single point
	void Set(const vector3<type>& point)
	{
		minCorner = point;
		maxCorner = point;
	}

	//! Adds a new point to the box
	/**
	The box grows bigger if the new point was outside the box
	\param X The x coordinate of the new point
	\param Y The y coordinate of the new point
	\param Z The z coordinate of the new point
	*/
	void AddPoint(type X, type Y, type Z)
	{
		if(X < minCorner.x) minCorner.x = X;
		if(Y < minCorner.y) minCorner.y = Y;
		if(Z < minCorner.z) minCorner.z = Z;
		if(X > maxCorner.x) maxCorner.x = X;
		if(Y > maxCorner.y) maxCorner.y = Y;
		if(Z > maxCorner.z) maxCorner.z = Z;
	}


	//! Adds a new point to the box
	/**
	The box grows bigger if the new point was outside the box
	\param point The point to add to the box
	*/
	void AddPoint(const vector3<type>& point)
	{
		AddPoint(point.x, point.y, point.z);
	}

	//! Adds a new box to the box
	/**
	The box grows bigger if the new box was outside the box
	\param other Bounding-box die umspannt werden soll
	*/
	void AddBox(const aabbox3d<type>& other)
	{
		AddPoint(other.minCorner);
		AddPoint(other.maxCorner);
	}

	//! Returns the center of the box
	/**
	\return The center of the box
	*/
	vector3<type> GetCenter() const
	{
		return (minCorner + maxCorner) / 2;
	}

	//! Return the extent of the box
	/**
	\return The extent vector of the box
	*/
	vector3<type> GetExtent() const
	{
		return maxCorner - minCorner;
	}

	//! Is the box "empty"
	/**
	\return True if Max is at the same place as Min
	*/
	bool IsEmpty() const
	{
		return maxCorner.Equal(minCorner);
	}

	//! Returns the Volume of the box
	/**
	\return The volume of the box
	*/
	type GetVolume() const
	{
		const vector3<type> e = GetExtent();
		return e.x * e.y * e.z;
	}

	//! Returns the Surface of the box
	/**
	\return The surface of the box
	*/
	type GetSurface() const
	{
		const vector3<type> e = GetExtent();
		return 2 * (e.x * e.z + e.x * e.y + e.z * e.y);
	}

	//! Gives the eight corners of the box
	/**
	\param out A array were the corners of the box are saved
	*/
	void GetCorners(vector3<type> out[8]) const
	{
		const vector3<type> Center = GetCenter();
		const vector3<type> Diag = Center - maxCorner;

		out[0] = vector3<type>(Center.x + Diag.x, Center.y + Diag.y, Center.z + Diag.z);
		out[1] = vector3<type>(Center.x + Diag.x, Center.y - Diag.y, Center.z + Diag.z);
		out[2] = vector3<type>(Center.x + Diag.x, Center.y + Diag.y, Center.z - Diag.z);
		out[3] = vector3<type>(Center.x + Diag.x, Center.y - Diag.y, Center.z - Diag.z);
		out[4] = vector3<type>(Center.x - Diag.x, Center.y + Diag.y, Center.z + Diag.z);
		out[5] = vector3<type>(Center.x - Diag.x, Center.y - Diag.y, Center.z + Diag.z);
		out[6] = vector3<type>(Center.x - Diag.x, Center.y + Diag.y, Center.z - Diag.z);
		out[7] = vector3<type>(Center.x - Diag.x, Center.y - Diag.y, Center.z - Diag.z);
	}

	//! Gives the six planes of the box
	/**
	\param out A array were the planes of the box are saved in order -X , +X, +Y, -Y, -Z, +Z
	*/
	void ComputePlanes(plane3d<type> out[6]) const
	{
		out[0].SetPlane(minCorner, vector3<type>(-1, 0, 0));
		out[1].SetPlane(maxCorner, vector3<type>(1, 0, 0));
		out[2].SetPlane(maxCorner, vector3<type>(0, 1, 0));
		out[3].SetPlane(minCorner, vector3<type>(0, -1, 0));
		out[4].SetPlane(minCorner, vector3<type>(0, 0, -1));
		out[5].SetPlane(maxCorner, vector3<type>(0, 0, 1));
	}

	//! Gives the nearest point to a given point, which is still inside the box.
	math::vector3<type> GetNearestPoint(const math::vector3<type>& v) const
	{
		return math::vector3<type>(
			math::Clamp(v.x, minCorner.x, maxCorner.x),
			math::Clamp(v.y, minCorner.y, maxCorner.y),
			math::Clamp(v.z, minCorner.z, maxCorner.z));
	}

	//! Ceck the box for errors and repairs them
	void Repair()
	{
		type t;

		if(minCorner.x > maxCorner.x) {
			t = minCorner.x; minCorner.x = maxCorner.x; maxCorner.x = t;
		}
		if(minCorner.y > maxCorner.y) {
			t = minCorner.y; minCorner.y = maxCorner.y; maxCorner.y = t;
		}
		if(minCorner.z > maxCorner.z) {
			t = minCorner.z; minCorner.z = maxCorner.z; maxCorner.z = t;
		}
	}

	//! Check a point for being inside the box(Inclusive Surface)
	/**
	\param point Point to check
	\return True, if the point is inside, otherwise False
	*/
	bool IsPointInside(const vector3<type>& point) const
	{
		return (point.x >= minCorner.x && point.x <= maxCorner.x &&
			point.y >= minCorner.y && point.y <= maxCorner.y &&
			point.z >= minCorner.z && point.z <= maxCorner.z);
	}

	//! Check a line for intersection with the box
	/**
	\param line The line to intersect with
	\return Was there a intersection
	*/
	bool IntersectWithLine(const line3d<type>& line) const
	{
		return IntersectWithLine(line.GetMiddle(), line.GetVector(), line.GetLength() / 2);
	}

	//! Check a line for intersection with the box
	/**
	\param linemiddle The middle of the line
	\param linevect The direction of the line
	\param halflength The half length of the line
	\return Was the an intersecition
	*/
	bool IntersectWithLine(const vector3<type>& linemiddle,
		const vector3<type>& linevect,
		type halflength) const
	{
		const vector3<type> e = GetExtent() / 2;
		const vector3<type> t = GetCenter() - linemiddle;

		if((fabs(t.x) > e.x + halflength * fabs(linevect.x)) ||
			(fabs(t.y) > e.y + halflength * fabs(linevect.y)) ||
			(fabs(t.z) > e.z + halflength * fabs(linevect.z)))
			return false;

		type r = e.y * (type)fabs(linevect.z) + e.z * (type)fabs(linevect.y);
		if(fabs(t.y*linevect.z - t.z*linevect.y) > r)
			return false;

		r = e.x * (type)fabs(linevect.z) + e.z * (type)fabs(linevect.x);
		if(fabs(t.z*linevect.x - t.x*linevect.z) > r)
			return false;

		r = e.x * (type)fabs(linevect.y) + e.y * (type)fabs(linevect.x);
		if(fabs(t.x*linevect.y - t.y*linevect.x) > r)
			return false;

		return true;
	}
};

//! Aliases for aabbox3d
typedef aabbox3d<float> aabbox3df;

template <typename T>
const aabbox3d<T> aabbox3d<T>::EMPTY = aabbox3d<T>(0, 0, 0, 0, 0, 0);

}    }    
#endif 