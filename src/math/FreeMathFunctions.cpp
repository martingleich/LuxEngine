#include "math/FreeMathFunctions.h"

namespace lux
{
namespace math
{

bool LineHitSphere(
	const Line3F& line,
	const Vector3F& center, float radius,
	LineSphereInfo* out)
{
	const Vector3F delta = line.start - center;
	const float distSq = delta.GetLengthSq();
	const float rSq = radius * radius;

	// Start is already inside the sphere -> abort
	if(distSq <= rSq) {
		if(out) {
			// Take the startpoint of the line as collision point
			// this isn't always exact, but will work for most collision
			// when the line isn't to deep inside the sphere.
			out->normal = delta.Normal();
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
	const Vector3F dir = line.GetVector();
	const float lenSq = dir.GetLengthSq();

	// u *(p-m)
	const float temp = dir.Dot(delta);

	// Diskriminante
	const float disc = temp * temp - lenSq * (distSq - rSq);

	if(disc < 0)
		return false;

	if(IsZero(disc)) {
		// Contact in single point.

		// Calculate the point on the line.
		const float s = -temp / lenSq;
		if(s < 0 || s > 1)
			return false;

		if(out) {
			out->distance = s;
			const Vector3F pos = line.start + s * dir;
			out->normal = (pos - center) / radius;
		}

		return true;
	} else {
		// Contact in two points.
		// const float s1 = (-temp - glm::sqrt(disc)) / lenSq;
		// const float s2 = (-temp + glm::sqrt(disc)) / lenSq;

		// Take the point nearer to the start.
		const float s = (-temp - (float)std::sqrt(disc)) / lenSq;
		if(s < 0 || s > 1)
			return false;

		if(out) {
			out->distance = s;
			const Vector3F pos = line.start + s * dir;
			out->normal = (pos - center).Normal();
		}

		return true;
	}
}

bool SphereHitSphere(
	const Vector3F& centerA, float radiusA,
	const Vector3F& centerB, float radiusB,
	SphereSphereInfo* out)
{
	const float r = radiusA + radiusB;

	const math::Vector3F delta = centerA - centerB;
	const float distanceSq = delta.GetLengthSq();
	if(distanceSq <= r * r) {
		if(out) {
			float distance = std::sqrt(distanceSq);
			out->seperation = delta / distance;
			out->penetration = r - distance;
			out->position = 0.5f*out->seperation + centerB;
		}
		return true;
	} else
		return false;
}

bool SphereHitBox(
	const math::Vector3F& center, float radius,
	const math::Vector3F& halfSize, const math::Transformation& trans,
	SphereBoxInfo* out)
{
	// Transform the sphere to box space.
	const math::Vector3F relCenter = trans.TransformInvPoint(center);
	const float relRadius = radius / trans.scale;

	// Get the next point in teh box to the sphere.
	const math::Vector3F nearest(
		math::Clamp(relCenter.x, -halfSize.x, halfSize.x),
		math::Clamp(relCenter.y, -halfSize.y, halfSize.y),
		math::Clamp(relCenter.z, -halfSize.z, halfSize.z));

	// If the distance from the center to this point is smaller than the radius of the
	// sphere, there is a collision.
	float distanceSq = nearest.GetDistanceToSq(relCenter);
	if(distanceSq <= relRadius * relRadius) {
		if(out) {
			float f = std::sqrt(distanceSq);
			if(nearest == relCenter) {
				out->seperation = GetUnitCubeVector(relCenter);
				if(out->seperation == math::Vector3F::ZERO)
					out->seperation = math::Vector3F::UNIT_X;
				else
					out->seperation.Normalize();
				const math::Vector3F nearestOnWall(
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

bool LineTestBox(
	const math::Line3F& line,
	const math::Vector3F& halfSize, const math::Transformation& trans)
{
	math::AABBoxF box(-halfSize, halfSize);

	math::Line3F transLine;
	trans.TransformObject(line, transLine);

	return IntersectAABoxWithLine(box, transLine);
}

namespace
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

bool LineHitBox(
	const math::Line3F& line,
	const math::Vector3F& halfSize, const math::Transformation& trans,
	LineBoxInfo* out)
{
	auto transLineStart = trans.TransformInvPoint(line.start);
	auto transLineVec = trans.TransformDir(line.GetVector());
	float len = transLineVec.GetLength();
	math::Vector3F dir = transLineVec / len;
	math::Vector3F origin = transLineStart;

	float t0 = -std::numeric_limits<float>::max();
	float t1 = std::numeric_limits<float>::max();

	if(
		Clip(+dir.x, -origin.x - halfSize.x, t0, t1) &&
		Clip(-dir.x, +origin.x - halfSize.x, t0, t1) &&
		Clip(+dir.y, -origin.y - halfSize.y, t0, t1) &&
		Clip(-dir.y, +origin.y - halfSize.y, t0, t1) &&
		Clip(+dir.z, -origin.z - halfSize.z, t0, t1) &&
		Clip(-dir.z, +origin.z - halfSize.z, t0, t1)) {

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
				math::Vector3F transPos = transLineStart + out->distance*transLineVec / len;
				transPos.x /= halfSize.x;
				transPos.y /= halfSize.y;
				transPos.z /= halfSize.z;
				out->normal = trans.TransformDir(GetUnitCubeVector(transPos)) / trans.scale;
				return true;
			}
		}
	}
	return false;
}

namespace
{
struct CollisionBox
{
	math::Vector3F axes[3];

	CollisionBox(const math::Vector3F& halfSize, const math::Transformation& trans)
	{
		axes[0] = halfSize.x * trans.TransformDir(math::Vector3F::UNIT_X);
		axes[1] = halfSize.y * trans.TransformDir(math::Vector3F::UNIT_Y);
		axes[2] = halfSize.z * trans.TransformDir(math::Vector3F::UNIT_Z);
	}
};

float TransformToAxis(
	const CollisionBox& box,
	const math::Vector3F& axis)
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
bool OverlapOnAxis(
	const CollisionBox& one,
	const CollisionBox& two,
	const math::Vector3F& axis,
	const math::Vector3F& toCenter)
{
	float oneProject = TransformToAxis(one, axis);
	float twoProject = TransformToAxis(two, axis);

	float distance = math::Abs(toCenter.Dot(axis));

	return (distance < oneProject + twoProject);
}

}

bool BoxTestBox(
	const math::Vector3F& halfSizeA, const math::Transformation& transA,
	const math::Vector3F& halfSizeB, const math::Transformation& transB)
{
	CollisionBox one(halfSizeA, transA);
	CollisionBox two(halfSizeB, transB);

	math::Vector3F toCenter = transB.translation - transA.translation;

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

bool TriangleTestSphere(
	const math::Vector3F& center,
	float radius,
	const math::Triangle3F& tri)
{
	// Translate world, so sphere is centered in origin.
	auto A = tri.A - center;
	auto B = tri.B - center;
	auto C = tri.C - center;

	auto rr = radius * radius;

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

bool IntersectPlaneWithLine(
	const PlaneF& plane,
	const Vector3F& lineBase,
	const Vector3F& lineDir,
	float& outSegment,
	bool isInfinite)
{
	const float denominator = plane.normal.Dot(lineDir);

	if(IsZero(denominator)) {
		// Line and plane are parallel
		if(IsZero(plane.normal.Dot(lineBase) + plane.d)) {
			// Line lies on the plane
			outSegment = 0.0f;
			return true;
		} else {
			return false;
		}
	}

	outSegment = -(plane.normal.Dot(lineBase) + plane.d) / denominator;
	if(!isInfinite) {
		if(outSegment < 0 || outSegment > 1)
			return false;
	}
	return true;
}
EPlaneRelation TestPlaneWithAABox(
	const PlaneF& plane,
	const Vector3F& boxMin,
	const Vector3F& boxMax)
{
	auto nearPoint = boxMax;
	auto farPoint = boxMin;
	if(plane.normal.x > 0) {
		nearPoint.x = boxMin.x;
		farPoint.x = boxMax.x;
	}
	if(plane.normal.y > 0) {
		nearPoint.y = boxMin.y;
		farPoint.y = boxMax.y;
	}
	if(plane.normal.z > 0) {
		nearPoint.z = boxMin.z;
		farPoint.z = boxMax.z;
	}
	if(plane.normal.Dot(nearPoint) > -plane.d)
		return EPlaneRelation::Front;
	if(plane.normal.Dot(farPoint) > -plane.d)
		return EPlaneRelation::Clipped;
	return EPlaneRelation::Back;
}

bool Intersect2Planes(
	const PlaneF& aPlane,
	const PlaneF& bPlane,
	Vector3F& outBase,
	Vector3F& outDir)
{
	const float fn01 = aPlane.normal.Dot(bPlane.normal);
	const float det = 1 - fn01 * fn01;

	if(IsZero(det))
		return false;

	const float invdet = 1 / det;
	const float fc0 = (-aPlane.d + fn01 * bPlane.d) * invdet;
	const float fc1 = (-bPlane.d + fn01 * aPlane.d) * invdet;

	outBase = aPlane.normal * fc0 + bPlane.normal*fc1;
	outDir = aPlane.normal.Cross(bPlane.normal);
	return true;
}


bool IntersectAABoxWithLine(
	const AABBoxF& box,
	const Vector3F& linemiddle,
	const Vector3F& linevect,
	float halflength)
{
	const Vector3F e = box.GetExtent() / 2;
	const Vector3F t = box.GetCenter() - linemiddle;

	if((fabs(t.x) > e.x + halflength * fabs(linevect.x)) ||
		(fabs(t.y) > e.y + halflength * fabs(linevect.y)) ||
		(fabs(t.z) > e.z + halflength * fabs(linevect.z)))
		return false;

	float r = e.y * (float)fabs(linevect.z) + e.z * (float)fabs(linevect.y);
	if(fabs(t.y*linevect.z - t.z*linevect.y) > r)
		return false;

	r = e.x * (float)fabs(linevect.z) + e.z * (float)fabs(linevect.x);
	if(fabs(t.z*linevect.x - t.x*linevect.z) > r)
		return false;

	r = e.x * (float)fabs(linevect.y) + e.y * (float)fabs(linevect.x);
	if(fabs(t.x*linevect.y - t.y*linevect.x) > r)
		return false;

	return true;
}

bool Intersect3Planes(
	const PlaneF& aPlane,
	const PlaneF& bPlane,
	const PlaneF& cPlane,
	Vector3F& outPoint)
{
	Vector3F lineBase;
	Vector3F lineDir;
	if(!Intersect2Planes(aPlane, bPlane, lineBase, lineDir))
		return false;

	float seg;
	if(!IntersectPlaneWithLine(cPlane, lineBase, lineDir, seg, true))
		return false;

	outPoint = lineBase + seg * lineDir;
	return true;
}

Vector3F GetNearestPointOnTriangleToPoint(const Triangle3F& tri, const Vector3F& point)
{
	Vector3F pointToEdgeAB = GetNearestPointOnLineFromPoint(Line3F(tri.A, tri.B), point);
	Vector3F pointToEdgeBC = GetNearestPointOnLineFromPoint(Line3F(tri.B, tri.C), point);
	Vector3F pointToEdgeAC = GetNearestPointOnLineFromPoint(Line3F(tri.A, tri.C), point);

	float toEdgeAB = pointToEdgeAB.GetDistanceToSq(point);
	float toEdgeBC = pointToEdgeBC.GetDistanceToSq(point);
	float toEdgeAC = pointToEdgeAC.GetDistanceToSq(point);

	if(toEdgeAB < toEdgeBC)
		return toEdgeAB < toEdgeAC ? pointToEdgeAB : pointToEdgeAC;

	return toEdgeBC < toEdgeAC ? pointToEdgeBC : pointToEdgeAC;
}

bool IntersectTriangleWithLine(const Triangle3F& tri, const Line3F& line, Vector3F& out)
{
	const Vector3F AB = tri.A - tri.B;
	const Vector3F BC = tri.B - tri.C;
	const Vector3F Normal = AB.Cross(BC);

	PlaneF plane = PlaneF(tri.A, Normal);
	float s;
	if(!IntersectPlaneWithLine(plane, line.start, line.GetVector(), s))
		return false;

	const Vector3F point = line.GetPoint(s);

	Vector3F subNormal = AB.Cross(Normal);
	if(point.Dot(subNormal) < tri.A.Dot(subNormal))
		return false;

	subNormal = BC.Cross(Normal);
	if(point.Dot(subNormal) < tri.B.Dot(subNormal))
		return false;

	subNormal = (tri.A - tri.C).Cross(Normal);
	if(point.Dot(subNormal) < tri.A.Dot(subNormal))
		return false;

	out = point;
	return true;
}

bool IntersectTriangleWithLineBary(const Triangle3F& tri, const Line3F& line, Vector3F& out)
{
	const Vector3F a(tri.C - tri.A);
	const Vector3F b(tri.B - tri.A);

	PlaneF plane = PlaneF(tri.A, a.Cross(b));
	float s;
	if(!IntersectPlaneWithLine(plane, line.start, line.GetVector(), s))
		return false;

	Vector3F c(line.GetPoint(s) - tri.A);

	float dotAA = a.Dot(a);
	float dotAB = a.Dot(b);
	float dotAC = a.Dot(c);
	float dotBB = b.Dot(b);
	float dotBC = b.Dot(c);

	float invDenom = 1 / (dotAA * dotBB - dotAB * dotAB);

	float u = (dotBB * dotAC - dotAB * dotBC) * invDenom;
	float v = (dotAA * dotBC - dotAB * dotAC) * invDenom;

	if(u > 0 && v > 0 && u + v < 1) {
		out = line.GetPoint(s);
		return true;
	}

	return false;
}

Vector3F GetNearestPointOnLineFromPoint(const Line3F& line, const Vector3F& point)
{
	Vector3F v(line.end - line.start);
	Vector3F w(point - line.start);

	float d = w.Dot(v) / v.GetLengthSq();
	if(d < 0)
		return line.start;
	else if(d > 0)
		return line.end;

	return line.start + v * d;
}

float GetDistanceFromLineToLine(const Line3<float>& aline, const Line3<float>& bline)
{
	Vector3F u = aline.end - aline.start;
	Vector3F v = bline.end - bline.start;
	Vector3F w = aline.start - bline.start;
	float a = u.GetLengthSq();
	float c = v.GetLengthSq();
	float b = u.Dot(v);
	float d = u.Dot(w);
	float e = v.Dot(w);
	float D = a * c - b * b;

	if(IsZero(D)) {
		float tc = b > c ? d / b : e / c;

		return (w - v * tc).GetLength();
	} else {
		float sc = (b*e - c * d) / D;
		float tc = (a*e - b * d) / D;

		return (w + u * sc - v * tc).GetLength();
	}
}

bool TestFrustumWithPoint(const ViewFrustum& frustum, const math::Vector3F& point)
{
	for(auto& p : frustum.Planes()) {
		if(TestPlaneWithPoint(p, point) == math::EPlaneRelation::Back)
			return false;
	}

	return true;
}

bool IsAABoxMaybeVisible(const ViewFrustum& frustum, const math::AABBoxF& box)
{
	for(auto& p : frustum.Planes()) {
		auto cls = TestPlaneWithAABox(p, box.minCorner, box.maxCorner);
		if(cls == EPlaneRelation::Back)
			return false;
	}

	return true;
}

bool IsOrientedBoxMaybeVisible(const ViewFrustum& frustum, const math::AABBoxF& box, const math::Transformation& boxTransform)
{
	auto invTransform = boxTransform.GetInverted();
	math::PlaneF tplane;
	for(auto& p : frustum.Planes()) {
		invTransform.TransformObject(p, tplane);
		auto cls = TestPlaneWithAABox(tplane, box.minCorner, box.maxCorner);
		if(cls == EPlaneRelation::Back)
			return false;
	}
	return true;
}

} // namespace math
} // namespace lux

