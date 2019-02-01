#ifndef INCLUDED_LUX_AABBOX3D_H
#define INCLUDED_LUX_AABBOX3D_H
#include "math/Plane.h"
#include "math/Vector3.h"

namespace lux
{
namespace math
{

//! Object for axis-aligned Bounding-Boxes
template <typename T>
class AABBox
{
public:
	//! The edge which is nearer to the (-,-,-)-Spacecube
	Vector3<T> minCorner;
	//! The edge which is nearer to the (+,+,+)-Spacecube
	Vector3<T> maxCorner;

	//! An empty box located at the origin
	static const AABBox<T> EMPTY;

	//! default-Constructor
	AABBox() : minCorner(0, 0, 0), maxCorner(0, 0, 0)
	{
	}
	//! from two edges
	AABBox(const Vector3<T>& min, const Vector3<T>& max) : minCorner(min), maxCorner(max)
	{
	}
	//! from a single point
	AABBox(const Vector3<T>& start) : minCorner(start), maxCorner(start)
	{
	}
	//! from two edges in single Coords
	AABBox(T minX, T minY, T minZ, T maxX, T maxY, T maxZ) : minCorner(minX, minY, minZ), maxCorner(maxX, maxY, maxZ)
	{
	}

	bool operator== (const AABBox<T>& other) const { return (minCorner == other.minCorner && maxCorner == other.maxCorner); }
	bool operator!= (const AABBox<T>& other) const { return !(*this == other); }

	//! Set the box to a single point
	void Set(T X, T Y, T Z)
	{
		minCorner.Set(X, Y, Z);
		maxCorner = minCorner;
	}

	//! Set the box to a single point
	void Set(const Vector3<T>& point)
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
	void AddPoint(T X, T Y, T Z)
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
	void AddPoint(const Vector3<T>& point)
	{
		AddPoint(point.x, point.y, point.z);
	}

	//! Adds a new box to the box
	/**
	The box grows bigger if the new box was outside the box
	\param other Bounding-box die umspannt werden soll
	*/
	void AddBox(const AABBox<T>& other)
	{
		AddPoint(other.minCorner);
		AddPoint(other.maxCorner);
	}

	//! Returns the center of the box
	/**
	\return The center of the box
	*/
	Vector3<T> GetCenter() const
	{
		return (minCorner + maxCorner) / 2;
	}

	//! Return the extent of the box
	/**
	\return The extent vector of the box
	*/
	Vector3<T> GetExtent() const
	{
		return maxCorner - minCorner;
	}

	//! Is the box "empty"
	/**
	\return True if Max is at the same place as Min
	*/
	bool IsEmpty() const
	{
		return math::IsEqual(maxCorner, minCorner);
	}

	//! Returns the Volume of the box
	/**
	\return The volume of the box
	*/
	T GetVolume() const
	{
		const Vector3<T> e = GetExtent();
		return e.x * e.y * e.z;
	}

	//! Returns the Surface of the box
	/**
	\return The surface of the box
	*/
	T GetSurface() const
	{
		const Vector3<T> e = GetExtent();
		return 2 * (e.x * e.z + e.x * e.y + e.z * e.y);
	}

	//! Ceck the box for errors and repairs them
	void Repair()
	{
		T t;

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
};

//! Aliases for aabbox3d
typedef AABBox<float> AABBoxF;

template <typename T>
const AABBox<T> AABBox<T>::EMPTY = AABBox<T>(0, 0, 0, 0, 0, 0);


//! Gives the eight corners of the box
/**
\param out A array were the corners of the box are saved in the following order
   3------7
  /|     /|
 / |    / |
2--+---6  |
|  1---+--5
| /    | /
|/     |/
0------4
*/
template <typename T>
void GetAABoxCorners(const AABBox<T>& box, Vector3<T> out[8])
{
	out[0] = Vector3<T>(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	out[1] = Vector3<T>(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	out[2] = Vector3<T>(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	out[3] = Vector3<T>(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	out[4] = Vector3<T>(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	out[5] = Vector3<T>(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	out[6] = Vector3<T>(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	out[7] = Vector3<T>(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
}

//! Gives the six planes of the box
/**
\param out A array were the planes of the box are saved in order -X , +X, +Y, -Y, -Z, +Z
*/
template <typename T>
void GetAABoxPlanes(const AABBox<T>& box, Plane<T> out[6])
{
	out[0].SetPlane(box.minCorner, Vector3<T>(-1, 0, 0));
	out[1].SetPlane(box.maxCorner, Vector3<T>(1, 0, 0));
	out[2].SetPlane(box.maxCorner, Vector3<T>(0, 1, 0));
	out[3].SetPlane(box.minCorner, Vector3<T>(0, -1, 0));
	out[4].SetPlane(box.minCorner, Vector3<T>(0, 0, -1));
	out[5].SetPlane(box.maxCorner, Vector3<T>(0, 0, 1));
}


} // namespace math
} // namespace lux

#endif 