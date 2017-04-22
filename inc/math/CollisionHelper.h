#ifndef INCLUDED_COLLISION_HELPER_H
#define INCLUDED_COLLISION_HELPER_H
#include "math/vector3.h"
#include "math/line3d.h"
#include "math/Transformation.h"

namespace lux
{
namespace math
{

// Algorithms are based on https://geometrictools.com/

template <typename T>
struct LineSphereInfo
{
	vector3<T> normal;
	T distance;
};

template <typename T>
bool LineHitSphere(
	const line3d<T>& line,
	const vector3<T>& center, T radius,
	LineSphereInfo<T>* out)
{
	const vector3<T> delta = line.start - center;
	const T distSq = delta.GetLengthSq();
	const T rSq = radius*radius;

	// Start is already inside the sphere -> abort
	if(distSq <= rSq) {
		if(out) {
			// Take the startpoint of the line as collision point
			// this isn't always exact, but will work for most collision
			// when the line isn't to deep inside the sphere.
			out->normal = delta.Normal_s();
			out->distance = 0;
		}
		return true;
	}

	/*
	Collision sphere-line:
	line-equation is l: x = p + s*u // Where p is the start point and u is the direction of the line
	sphere-equation is s: (x-m)² = r² // Where m is the center and r is the radius of the sphere
	Resolving the power in s gives:
	s: x² - 2xm + m² = r²
	Putting l into this yields:
	(p+s*u)² - 2(p+s*u)*m + m² = r²
	Simplifing this gives:
	s²+s*(2u(p-m))/u² + ((p-m)² - r²)/u² = 0
	When setting:
	a = 1;
	b = (2u(p-m))/u²;
	c = ((p-m)² - r²)/u²;
	The solution for s is
	s = (-b +- sqrt(b²-4c)) / 2a
	*/
	const vector3<T> dir = line.GetVector();
	const T lenSq = dir.GetLengthSq();

	// u *(p-m)
	const T temp = dir.Dot(delta);

	// Diskriminante
	const T disc = temp * temp - lenSq *(distSq - rSq);

	if(disc < 0)
		return false;

	if(IsZero(disc)) {
		// Contact in single point.

		// Calculate the point on the line.
		const T s = -temp / lenSq;
		if(s < 0 || s > 1)
			return false;

		if(out) {
			out->distance = s;
			const vector3<T> pos = line.start + s*dir;
			out->normal = (pos - center) / radius;
		}

		return true;
	} else {
		// Contact in two points.
		// const float s1 = (-temp - glm::sqrt(disc)) / lenSq;
		// const float s2 = (-temp + glm::sqrt(disc)) / lenSq;

		// Take the point nearer to the start.
		const T s = (-temp - sqrtf(disc)) / lenSq;
		if(s < 0 || s > 1)
			return false;

		if(out) {
			out->distance = s;
			const vector3<T> pos = line.start + s*dir;
			out->normal = (pos-center).Normal_s();
		}

		return true;
	}
}

template <typename T>
struct SphereSphereInfo
{
	vector3<T> position;
	vector3<T> seperation;
	T penetration;
};

template <typename T>
struct SphereBoxInfo
{
	vector3<T> position;
	vector3<T> seperation;
	T penetration;
};

template <typename T>
bool SphereHitSphere(
	const vector3<T>& centerA, T radiusA,
	const vector3<T>& centerB, T radiusB,
	SphereSphereInfo<T>* out = nullptr)
{
	const T r = radiusA + radiusB;

	const math::vector3<T> delta = centerB - centerA;
	const T distanceSq = delta.GetLengthSq();
	if(distanceSq <= r*r) {
		if(out) {
			T distance = sqrt(distanceSq);
			out->seperation = delta / distance;
			out->penetration = r - distance;
			out->position = 0.5f*out->seperation + centerA;
		}
		return true;
	} else
		return false;
}

template <typename T>
bool SphereHitBox(
	const math::vector3<T>& center, T radius,
	const math::vector3<T>& halfSize, const math::Transformation& trans,
	SphereBoxInfo<T>* out = nullptr)
{
	// Transform the sphere to box space.
	const math::vector3<T> relCenter = trans.TransformInvPoint(center);
	const T relRadius = radius / trans.scale;

	// Get the next point in teh box to the sphere.
	const math::vector3<T> nearest(
		math::Clamp(relCenter.x, -halfSize.x, halfSize.x),
		math::Clamp(relCenter.y, -halfSize.y, halfSize.y),
		math::Clamp(relCenter.z, -halfSize.z, halfSize.z));

	// If the distance from the center to this point is smaller than the radius of the
	// sphere, there is a collision.
	T distanceSq = nearest.GetDistanceToSq(relCenter);
	if(distanceSq <= relRadius*relRadius) {
		if(out) {
			T f = sqrt(distanceSq);
			out->seperation = trans.TransformDir((relCenter - nearest) / f);
			out->penetration = (radius - f) * trans.scale;
			out->position = trans.TransformPoint(nearest) - 0.5f * out->seperation;
		}
		return true;
	}
	return false;
}

template <typename T>
bool PointTestBox(
	const math::vector3<T>& point,
	const math::vector3<T>& halfSize, const math::Transformation& trans)
{
	math::vector3<T> p = trans.TransformInvPoint(point);
	if(abs(p.x) > halfSize.x || abs(p.y) > halfSize.y || abs(p.z) > halfSize.z)
		return false;
	else
		return true;
}

template <typename T>
struct LineBoxInfo
{
	math::vector3<T> normal;
	T distance;
};

template <typename T>
bool LineTestBox(
	const math::line3d<T>& line,
	const math::vector3<T>& halfSize, const math::Transformation& trans)
{
	math::aabbox3d<T> box(-halfSize, halfSize);

	math::line3d<T> transLine;
	trans.TransformObject(line, transLine);

	return box.IntersectWithLine(transLine);
}

namespace implCollision
{
template <typename T>
bool Clip(T denom, T numer, T& t0, T& t1)
{
	if(denom > 0) {
		if(numer > denom*t1)
			return false;
		if(numer > denom*t0)
			t0 = numer / denom;
		return true;
	} else if(denom < 0) {
		if(numer > denom*t0)
			return false;
		if(numer > denom*t1)
			t1 = numer / denom;
		return true;
	} else
		return numer <= 0;
}
}

template <typename T>
bool LineHitBox(
	const math::line3d<T>& line,
	const math::vector3<T>& halfSize, const math::Transformation& trans,
	LineBoxInfo<T>* out)
{
	math::line3d<T> transLine = math::line3d<T>(trans.TransformInvPoint(line.start), trans.TransformInvPoint(line.end));
	T len = transLine.GetVector().GetLength();
	math::vector3<T> dir = transLine.GetVector() / len;
	math::vector3<T> origin = transLine.start;

	T t0 = -std::numeric_limits<T>::max();
	T t1 = std::numeric_limits<T>::max();

	if(
		implCollision::Clip(+dir.x, -origin.x - halfSize.x, t0, t1) &&
		implCollision::Clip(-dir.x, +origin.x - halfSize.x, t0, t1) &&
		implCollision::Clip(+dir.y, -origin.y - halfSize.y, t0, t1) &&
		implCollision::Clip(-dir.y, +origin.y - halfSize.y, t0, t1) &&
		implCollision::Clip(+dir.z, -origin.z - halfSize.z, t0, t1) &&
		implCollision::Clip(-dir.z, +origin.z - halfSize.z, t0, t1)) {

		if(t1 > t0) {
			bool found = false;
			if(t0 >= 0 && t0 <= len) {
				if(out) {
					out->distance = t0;
					found = true;
				}
			} else if(t0 <= 0 && t1 > 0) {
				if(out) {
					out->distance = 0;
					found = true;
				}
			}

			if(found) {
				math::vector3<T> transPos = transLine.start + out->distance*transLine.GetVector() / len;
				transPos.x /= halfSize.x;
				transPos.y /= halfSize.y;
				transPos.z /= halfSize.z;
				out->normal = trans.TransformDir(transPos.GetUnitCubeVector());
				return true;
			}
		}
	}
	return false;
}

namespace implCollision
{
template <typename T>
struct CollisionBox
{
	math::vector3<T> axes[3];
	math::vector3<T> center;

	CollisionBox(const math::vector3<T>& halfSize, const math::Transformation& trans)
	{
		center = trans.TransformPoint(math::vector3<T>::ZERO);
		axes[0] = trans.TransformDir(math::vector3<T>(halfSize.x, 0, 0));
		axes[1] = trans.TransformDir(math::vector3<T>(0, halfSize.y, 0));
		axes[2] = trans.TransformDir(math::vector3<T>(0, 0, halfSize.z));
	}
};

template <typename T>
T TransformToAxis(
	const CollisionBox<T>& box,
	const math::vector3<T>& axis)
{
	return
		math::Abs(axis.Dot(box.axes[0])) +
		math::Abs(axis.Dot(box.axes[1])) +
		math::Abs(axis.Dot(box.axes[2]));
}

//! Check for box overlap on given axis.
/**
\param one The first box
\param two The second box
\param toCentre The distance vector between the boxes.
\param axis The axis to test
*/
template <typename T>
bool OverlapOnAxis(
	const CollisionBox<T>& one,
	const CollisionBox<T>& two,
	const math::vector3<T>& toCentre,
	const math::vector3<T>& axis)
{
	T oneProject = TransformToAxis(one, axis);
	T twoProject = TransformToAxis(two, axis);

	T distance = math::Abs(toCentre.Dot(axis));

	return (distance < oneProject + twoProject);
}

template <typename T>
bool BoxTestBox(
	const CollisionBox<T>& one,
	const CollisionBox<T>& two)
{
	math::vector3<T> toCentre = two.center - one.center;

	return
		OverlapOnAxis(one, two, one.axes[0], toCentre) &&
		OverlapOnAxis(one, two, one.axes[1], toCentre) &&
		OverlapOnAxis(one, two, one.axes[2], toCentre) &&

		OverlapOnAxis(one, two, two.axes[0], toCentre) &&
		OverlapOnAxis(one, two, two.axes[1], toCentre) &&
		OverlapOnAxis(one, two, two.axes[2], toCentre) &&

		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[0]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[1]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[2]), toCentre) &&


		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[0]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[1]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[2]), toCentre) &&

		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[0]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[1]), toCentre) &&
		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[2]), toCentre);
}
}

template <typename T>
bool BoxTestBox(
	const math::vector3<T>& halfSizeA, const math::Transformation& transA,
	const math::vector3<T>& halfSizeB, const math::Transformation& transB)
{
	implCollision::CollisionBox<T> a(halfSizeA, transA);
	implCollision::CollisionBox<T> b(halfSizeB, transB);

	return BoxTestBox(a, b);
}

}
}

#endif // #ifndef INCLUDED_COLLISION_HELPER_H