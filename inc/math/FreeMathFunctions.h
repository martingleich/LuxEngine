#ifndef INCLUDED_LUX_FREE_MATH_FUNCTIONS_H
#define INCLUDED_LUX_FREE_MATH_FUNCTIONS_H
#include "math/Vector3.h"
#include "math/Line3.h"
#include "math/Transformation.h"
#include "math/ViewFrustum.h"

namespace lux
{
namespace math
{

enum class EPlaneRelation
{
	Front = 0,       //!< The object is before the plane    
	Back,            //!< The object is behind the plane
	Plane,           //!< The object is on the plane
	Clipped          //!< The object is cut by the plane
};

struct LineSphereInfo
{
	Vector3F normal;
	float distance;
};

LUX_API bool LineHitSphere(
	const Line3F& line,
	const Vector3F& center, float radius,
	LineSphereInfo* out);

struct SphereSphereInfo
{
	Vector3F position;
	Vector3F seperation;
	float penetration;
};

struct SphereBoxInfo
{
	Vector3F position;
	Vector3F seperation;
	float penetration;
};

LUX_API bool SphereHitSphere(
	const Vector3F& centerA, float radiusA,
	const Vector3F& centerB, float radiusB,
	SphereSphereInfo* out = nullptr);

LUX_API bool SphereHitBox(
	const math::Vector3F& center, float radius,
	const math::Vector3F& halfSize, const math::Transformation& trans,
	SphereBoxInfo* out = nullptr);

inline bool PointTestBox(
	const math::Vector3F& point,
	const math::Vector3F& halfSize, const math::Transformation& trans)
{
	math::Vector3F p = trans.TransformInvPoint(point).Absolute();
	return !(p.x > halfSize.x || p.y > halfSize.y || p.z > halfSize.z);
}

struct LineBoxInfo
{
	math::Vector3F normal;
	float distance;
};

LUX_API bool LineTestBox(
	const math::Line3F& line,
	const math::Vector3F& halfSize, const math::Transformation& trans);

LUX_API bool LineHitBox(
	const math::Line3F& line,
	const math::Vector3F& halfSize, const math::Transformation& trans,
	LineBoxInfo* out);

LUX_API bool BoxTestBox(
	const math::Vector3F& halfSizeA, const math::Transformation& transA,
	const math::Vector3F& halfSizeB, const math::Transformation& transB);

LUX_API bool TriangleTestSphere(
	const math::Vector3F& center,
	float radius,
	const math::Triangle3F& tri);

LUX_API bool IntersectPlaneWithLine(
	const PlaneF& plane,
	const Vector3F& lineBase, const Vector3F& lineDir, float& outSegment,
	bool isInfinite = false);

//! Compute the minimal distance to a point
/**
\param p Point to which the distance should computed
\return The distance to the point
*/
inline float GetDistanceFromPlaneToPoint(const PlaneF& plane, const Vector3F& point)
{
	return point.Dot(plane.normal) + plane.d;
}

inline bool TestLineWithPlane(const PlaneF& plane, const Line3F& line)
{
	float d1 = GetDistanceFromPlaneToPoint(plane, line.start);
	float d2 = GetDistanceFromPlaneToPoint(plane, line.end);

	// Both point are on diffrent sides of the plane, so there must be an intersection
	return (d1 <= 0 && d2 >= 0) || (d1 >= 0 && d2 <= 0);
}

//! Classify point relation to plane
inline EPlaneRelation TestPlaneWithPoint(const PlaneF& plane, const Vector3F& point)
{
	auto f = plane.normal.Dot(point);
	if(f < -plane.d)
		return EPlaneRelation::Back;
	if(f > -plane.d)
		return EPlaneRelation::Front;
	return EPlaneRelation::Plane;
}

EPlaneRelation TestPlaneWithAABox(const PlaneF& plane, const Vector3F& boxMin, const Vector3F& boxMax);

LUX_API bool Intersect2Planes(
	const PlaneF& aPlane, const PlaneF& bPlane,
	Vector3F& outBase, Vector3F& outDir);

//! Check a point for being inside the box(Inclusive Surface)
/**
\param point Point to check
\return True, if the point is inside, otherwise False
*/
inline bool TestAABoxWithPoint(const AABBoxF &box, const Vector3F& point)
{
	return (
		point.x >= box.minCorner.x && point.x <= box.maxCorner.x &&
		point.y >= box.minCorner.y && point.y <= box.maxCorner.y &&
		point.z >= box.minCorner.z && point.z <= box.maxCorner.z);
}

//! Gives the nearest point to a given point, which is still inside the box.
inline Vector3F GetNearestPointToAABox(const AABBoxF& box, const Vector3F& v)
{
	return Vector3F(
		Clamp(v.x, box.minCorner.x, box.maxCorner.x),
		Clamp(v.y, box.minCorner.y, box.maxCorner.y),
		Clamp(v.z, box.minCorner.z, box.maxCorner.z));
}

//! Check a line for intersection with the box
/**
\param linemiddle The middle of the line
\param linevect The direction of the line
\param halflength The half length of the line
\return Was the an intersecition
*/
LUX_API bool IntersectAABoxWithLine(
	const AABBoxF& box,
	const Vector3F& linemiddle, const Vector3F& linevect, float halflength);

//! Check a line for intersection with the box
/**
\param line The line to intersect with
\return Was there a intersection
*/
inline bool IntersectAABoxWithLine(const AABBoxF& box, const Line3F& line)
{
	return IntersectAABoxWithLine(box, line.GetMiddle(), line.GetVector(), line.GetLength() / 2);
}

LUX_API bool Intersect3Planes(
	const PlaneF& aPlane, const PlaneF& bPlane, const PlaneF& cPlane,
	Vector3F& outPoint);

//! Determinates if the triangle is totally inside a bounding box
/**
\param box The box test against
\return Is the triangle totally inside the box
*/
inline bool IsTotalInsideTriangleWithAABox(const Triangle3F& tri, const AABBoxF& box)
{
	return (
		TestAABoxWithPoint(box, tri.A) &&
		TestAABoxWithPoint(box, tri.B) &&
		TestAABoxWithPoint(box, tri.C));
}

//! Get the closest point on a triangle to another point
/**
\param point Point which must be on the same plane as the triangle
\return The nearest point on the triangle
*/
LUX_API Vector3F GetNearestPointOnTriangleToPoint(const Triangle3F& tri, const Vector3F& point);

//! Check if a line intersect with the triangle(include border)
/**
\param line Line to check
\param out If != NULL the coords of the intersection are written there
\return True if there is a intersection otherwise false
*/
LUX_API bool IntersectTriangleWithLine(const Triangle3F& tri, const Line3F& line, Vector3F& out);

//! Check if a line intersect with the triangle(including border)
/**
Uses a barycentric approach, is faster but less exact
\param line Line to check
\param out If != NULL the coords of the intersection are written there
\return True if there is a intersection otherwise false
*/
LUX_API bool IntersectTriangleWithLineBary(const Triangle3F& tri, const Line3F& line, Vector3F& out);

//! Check if a point is on the line
/**
\param p The point to check, this point must be kolinear with the line
\return Is the point between start and end
*/
inline bool IsPointOnLine(const Line3F& line, const Vector3F& p)
{
	return p.IsBetweenPoints(line.start, line.end);
}

//! The nearest point on the line
/**
\param Point The point to compare with
\return The nearest point to the param, which is on the line
*/
LUX_API Vector3F GetNearestPointOnLineFromPoint(const Line3F& line, const Vector3F& point);

//! Retrieves the distance to another line
/**
\param other The other line
\return The distance to the other line
*/
LUX_API float GetDistanceFromLineToLine(const Line3<float>& aline, const Line3<float>& bline);

//! Check if a point is inside the frustum
LUX_API bool TestFrustumWithPoint(const ViewFrustum& frustum, const math::Vector3F& point);

//! Returns if a axis aligned bounding box is maybe visible inside a frustum.
/*
If false is returned the box is never visible.
*/
LUX_API bool IsAABoxMaybeVisible(const ViewFrustum& frustum, const math::AABBoxF& box);

//! Returns if a object oriented bounding box is maybe visible inside a frustum.
/*
If false is returned the box is never visible.
*/
LUX_API bool IsOrientedBoxMaybeVisible(const ViewFrustum& frustum, const math::AABBoxF& box, const math::Transformation& boxTransform);

}
}

#endif // #ifndef INCLUDED_LUX_FREE_MATH_FUNCTIONS_H