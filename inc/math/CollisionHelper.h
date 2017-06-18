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
			out->normal = (pos - center).Normal_s();
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

	const math::vector3<T> delta = centerA - centerB;
	const T distanceSq = delta.GetLengthSq();
	if(distanceSq <= r*r) {
		if(out) {
			T distance = sqrt(distanceSq);
			out->seperation = delta / distance;
			out->penetration = r - distance;
			out->position = 0.5f*out->seperation + centerB;
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
			if(nearest == relCenter) {
				out->seperation = relCenter.GetUnitCubeVector();
				if(out->seperation == math::vector3<T>::ZERO)
					out->seperation = math::vector3<T>::UNIT_X;
				else
					out->seperation.Normalize();
				const math::vector3<T> nearestOnWall(
					relCenter.x < 0 ? -halfSize.x : halfSize.x,
					relCenter.y < 0 ? -halfSize.y : halfSize.y,
					relCenter.z < 0 ? -halfSize.z : halfSize.z);
				out->penetration = nearestOnWall.GetDistanceTo(relCenter) * trans.scale;
			} else {
				out->seperation = trans.TransformDir((relCenter - nearest) / f) / trans.scale;
				out->penetration = (relRadius - f) * trans.scale;
			}

			out->position = trans.TransformPoint(nearest) - 0.5f * out->seperation*out->penetration;
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

namespace impl_collision
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
		impl_collision::Clip(+dir.x, -origin.x - halfSize.x, t0, t1) &&
		impl_collision::Clip(-dir.x, +origin.x - halfSize.x, t0, t1) &&
		impl_collision::Clip(+dir.y, -origin.y - halfSize.y, t0, t1) &&
		impl_collision::Clip(-dir.y, +origin.y - halfSize.y, t0, t1) &&
		impl_collision::Clip(+dir.z, -origin.z - halfSize.z, t0, t1) &&
		impl_collision::Clip(-dir.z, +origin.z - halfSize.z, t0, t1)) {

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
				out->normal = trans.TransformDir(transPos.GetUnitCubeVector()) / trans.scale;
				return true;
			}
		}
	}
	return false;
}

namespace impl_collision
{
template <typename T>
struct CollisionBox
{
	math::vector3<T> axes[3];
	math::vector3<T> center;

	CollisionBox(const math::vector3<T>& halfSize, const math::Transformation& trans)
	{
		center = trans.TransformPoint(math::vector3<T>::ZERO);
		axes[0] = halfSize.x * trans.TransformDir(math::vector3<T>::UNIT_X);
		axes[1] = halfSize.y * trans.TransformDir(math::vector3<T>::UNIT_Y);
		axes[2] = halfSize.z * trans.TransformDir(math::vector3<T>::UNIT_Z);
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
\param toCenter The distance vector between the boxes.
\param axis The axis to test
*/
template <typename T>
bool OverlapOnAxis(
	const CollisionBox<T>& one,
	const CollisionBox<T>& two,
	const math::vector3<T>& axis,
	const math::vector3<T>& toCenter)
{
	T oneProject = TransformToAxis(one, axis);
	T twoProject = TransformToAxis(two, axis);

	T distance = math::Abs(toCenter.Dot(axis));

	return (distance < oneProject + twoProject);
}

template <typename T>
bool BoxTestBox(
	const CollisionBox<T>& one,
	const CollisionBox<T>& two)
{
	math::vector3<T> toCenter = two.center - one.center;

	return
		OverlapOnAxis(one, two, one.axes[0], toCenter) &&
		OverlapOnAxis(one, two, one.axes[1], toCenter) &&
		OverlapOnAxis(one, two, one.axes[2], toCenter) &&

		OverlapOnAxis(one, two, two.axes[0], toCenter) &&
		OverlapOnAxis(one, two, two.axes[1], toCenter) &&
		OverlapOnAxis(one, two, two.axes[2], toCenter) &&

		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[0]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[1]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[0].Cross(two.axes[2]), toCenter) &&

		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[0]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[1]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[1].Cross(two.axes[2]), toCenter) &&

		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[0]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[1]), toCenter) &&
		OverlapOnAxis(one, two, one.axes[2].Cross(two.axes[2]), toCenter);
}
}

template <typename T>
bool BoxTestBox(
	const math::vector3<T>& halfSizeA, const math::Transformation& transA,
	const math::vector3<T>& halfSizeB, const math::Transformation& transB)
{
	impl_collision::CollisionBox<T> a(halfSizeA, transA);
	impl_collision::CollisionBox<T> b(halfSizeB, transB);

	return BoxTestBox(a, b);
}


template <typename T>
bool TriangleTestSphere(
	const math::vector3<T>& center,
	T radius,
	const math::triangle3d<T>& tri)
{
	// Translate world, so sphere is centered in origin.
	auto A = tri.A - center;
	auto B = tri.B - center;
	auto C = tri.C - center;

	auto rr = radius*radius;

	auto AB = B - A;
	auto BC = C - B;
	auto CA = A - C;

	auto normal = (AB).Cross(-CA);
	auto d = A.Dot(normal);
	auto e = normal.GetLengthSq();
	// d*d/e is the distance of the sphere center to the triangle plane.
	if(d*d > rr*e)
		return false;

	// Seperating-Axis-Test
	auto aa = A.Dot(A);
	auto ab = A.Dot(B);
	auto ac = A.Dot(C);
	auto bb = B.Dot(B);
	auto bc = B.Dot(C);
	auto cc = C.Dot(C);

	if(aa > rr && ab > aa && ac > aa)
		return false;
	if(bb > rr && ab > bb && bc > bb)
		return false;
	if(cc > rr && ac > cc && bc > cc)
		return false;

	auto d1 = ab - aa;
	auto d2 = bc - bb;
	auto d3 = ac - cc;

	auto e1 = AB.GetLengthSq();
	auto e2 = BC.GetLengthSq();
	auto e3 = CA.GetLengthSq();

	auto Q1 = A * e1 - d1 * AB;
	auto Q2 = B * e2 - d2 * BC;
	auto Q3 = C * e3 - d3 * CA;
	auto QC = C * e1 - Q1;
	auto QA = A * e2 - Q2;
	auto QB = B * e3 - Q3;

	if(Q1.GetLengthSq() > rr * e1 * e1 && Q1.Dot(QC) > 0)
		return false;
	if(Q2.GetLengthSq() > rr * e2 * e2 && Q2.Dot(QA) > 0)
		return false;
	if(Q3.GetLengthSq() > rr * e3 * e3 && Q3.Dot(QB) > 0)
		return false;

	return true;
}


}
}

#endif // #ifndef INCLUDED_COLLISION_HELPER_H