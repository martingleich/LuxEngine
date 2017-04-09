#ifndef INCLUDED_PLANE3D_H
#define INCLUDED_PLANE3D_H
#include "line3d.h"

namespace lux
{
namespace math
{

//! Object for planes in 3D
template <typename type>
class plane3d
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
	vector3<type> Normal;
	type D;

	//! default constructor the XZ-Plane
	plane3d() : Normal(0, 1, 0), D(0)
	{
	}
	//! Construct from Normal and d value
	plane3d(const vector3<type>& _Normal, const type d) : Normal(_Normal), D(d)
	{
	}
	//! Construct from a point on the plane and the normal
	plane3d(const vector3<type>& Point, const vector3<type>& _Normal) : Normal(_Normal)
	{
		RecalculateD(Point);
	}
	//! Constructor from three points on the plane
	plane3d(const vector3<type>& A, const vector3<type>& B, const vector3<type>& C)
	{
		SetPlane(A, B, C);
	}

	//! Copyconstuctor
	plane3d(const plane3d& other) :
		Normal(other.Normal),
		D(other.D)
	{
	}

	//! Assignment
	plane3d<type> operator= (const plane3d<type> other)
	{
		Normal = other.Normal; D = other.D; return *this;
	}

	//! Equality
	inline bool operator== (const plane3d<type>& other) const
	{
		return Equal(D, other.D) && Normal == other.Normal;
	}

	//! Unequality
	inline bool operator!= (const plane3d<type>& other) const
	{
		return !(*this == other);
	}

	//! Construct a plane from normalvector and a memberpoint
	/**
	\param point A memberpoint of the plane
	\param _Normal The normalvector of the plane
	*/
	void SetPlane(const vector3<type>& point, const vector3<type>& _Normal)
	{
		Normal = _Normal;
		RecalculateD(point);
	}

	//! Construct a plane from Normalvektor and D
	/**
	\param _Normal Normalvector of the plane
	\param d D value of the plane
	*/
	void SetPlane(const vector3<type>& _Normal, type d)
	{
		Normal = _Normal;
		D = d;
	}

	//! Construct a plane from 3 Points
	/**
	\param A First point on the plane
	\param B Second point on the plane
	\param C Third point on the plane
	*/
	void SetPlane(const vector3<type>& A, const vector3<type>& B, const vector3<type>& C)
	{
		Normal = (B - A).Cross(C - A);
		Normal.Normalize_s();
		RecalculateD(A);
	}

	//! Rebuild the plane for a new memberpoint
	/**
	\param point New Member point to apply
	*/
	void RecalculateD(const vector3<type>& point)
	{
		D = -point.Dot(Normal);
	}

	//! Normalize the plane
	/**
	After this operation the Normalvector of the plane has the length 1
	\return A reference to the current plane
	*/
	plane3d<type>& Normalize()
	{
		const double dInvLength = 1.0 / Normal.GetLength();
		Normal *= dInvLength;
		D *= dInvLength;
		return *this;
	}

	//! Return any point which lies on the plane
	/**
	\return A point located on the plane
	*/
	vector3<type> GetMemberPoint() const
	{
		return Normal * -D;
	}

	//! Compute the minimal distance to a point
	/**
	\param p Point to which the distance should computed
	\return The distance to the point
	*/
	type GetDistanceTo(const vector3<type>& p) const
	{
		type out = p.Dot(Normal) + D;
		float lsq = Normal.GetLengthSq();
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
	bool IntersectWithLine(const line3d<type>& line,
		float* out = nullptr) const
	{
		const float denominator = Normal.Dot(line.GetVector());

		if(IsZero(denominator)) {
			// Line and plane are parallel
			if(IsZero(Normal.Dot(line.start) + D)) {
				// Line lies on the plane
				if(out) *out = 0.0f;
				return true;
			} else {
				return false;
			}
		}

		const float segment = -(Normal.Dot(line.start) + D) / denominator;

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
	bool IntesectWithLineFast(const line3d<type>& line) const
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
typedef plane3d<float> plane3df;

}    

}    


#endif