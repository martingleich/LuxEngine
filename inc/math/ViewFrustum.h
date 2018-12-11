#ifndef INCLUDED_LUX_MATH_VIEW_FRUSTUM_H
#define INCLUDED_LUX_MATH_VIEW_FRUSTUM_H
#include "math/AABBox.h"
#include "math/Matrix4.h"
#include "math/Plane.h"
#include "math/Transformation.h"
#include "core/lxIterator.h"

namespace lux
{
namespace math
{

//! Represent a camera viewfrustum
/*
A view frustum is defined by 6 planes, which intersect in 8 corners.
Order or planes: Near, Far, Left, Right, Top, Bottom
Order of corners: NearLeftUp, NearRightUp, NearRightDown, NearLeftDown,
	FarLeftUp, FarRightUp, FarRightDown, FarLeftDown
All normals are pointing inwards.
*/
class ViewFrustum
{
public:
	enum EPlane
	{
		Near = 0,
		Far = 1,
		Left = 2,
		Right = 3,
		Top = 4,
		Bottom = 5,
		Count = 6,
	};

	enum ERelation
	{
		Outside,
		Clipped,
		Inside,
	};

public:
	//! Create invalid view frustum
	ViewFrustum()
	{
	}

	//! Create frustum from planes
	ViewFrustum(math::PlaneF planes[Count])
	{
		for(int i = 0; i < Count; ++i) {
			m_Planes[i] = planes[i];
			m_Planes[i].Normalize();
		}

		RecalculateInternals();
	}

	//! Create view frustum from perspective camera
	/**
	\param camPos The position of the camera
	\param view The view matrix used by the camera
	\param fovY The vertical field of view in rad.
	\param aspect The aspect ratio(screenWidth/screenHeight)
	\param nearPlane The distance of the near clipping plane
	\param farPlane The distance of the far clipping plane
	\return The generated view frustum
	*/
	static ViewFrustum FromPerspCam(
		const math::Vector3F& camPos,
		const math::Matrix4& view,
		math::AngleF fovY, float aspect,
		float nearPlane, float farPlane)
	{
		math::Vector3F camRight(view(0, 0), view(1, 0), view(2, 0));
		camRight.Normalize();
		math::Vector3F camUp(view(0, 1), view(1, 1), view(2, 1));
		camUp.Normalize();
		math::Vector3F camDir(view(0, 2), view(1, 2), view(2, 2));
		camDir.Normalize();

		ViewFrustum out;
		// Near
		out.m_Planes[0].SetPlane(camPos + camDir * nearPlane, camDir);
		// Far
		out.m_Planes[1].SetPlane(camPos + camDir * farPlane, -camDir);
		// Left
		float thorz = math::Tan(fovY*0.5f)*aspect;
		auto leftSide = camDir - thorz * camRight;
		auto leftNormal = camUp.Cross(leftSide).Normal();
		out.m_Planes[2].SetPlane(camPos, leftNormal);
		// Right
		auto rightSide = camDir + thorz * camRight;
		auto rightNormal = rightSide.Cross(camUp).Normal();
		out.m_Planes[3].SetPlane(camPos, rightNormal);
		// Top
		float tvert = math::Tan(fovY*0.5f);
		auto topSide = camDir + tvert * camUp;
		auto topNormal = camRight.Cross(topSide).Normal();
		out.m_Planes[4].SetPlane(camPos, topNormal);
		// Bottom
		auto bottomSide = camDir - tvert * camUp;
		auto bottomNormal = bottomSide.Cross(camRight).Normal();
		out.m_Planes[5].SetPlane(camPos, bottomNormal);

		out.RecalculateInternals();

		return out;
	}

	//! Create view frustum from orthogonal camera
	/**
	\param camPos The position of the camera
	\param view The view matrix used by the camera
	\param xMax the maximal visible x offset
	\param aspect The aspect ratio(screenWidth/screenHeight)
	\param nearPlane The distance of the near clipping plane
	\param farPlane The distance of the far clipping plane
	\return The generated view frustum
	*/
	static ViewFrustum FromOrthoCam(
		const math::Vector3F& camPos,
		const math::Matrix4& view,
		float xMax, float aspect,
		float nearPlane, float farPlane)
	{
		float yMax = xMax / aspect;
		math::Vector3F camRight(view(0, 0), view(1, 0), view(2, 0));
		camRight.Normalize();
		math::Vector3F camUp(view(0, 1), view(1, 1), view(2, 1));
		camUp.Normalize();
		math::Vector3F camDir(view(0, 2), view(1, 2), view(2, 2));
		camDir.Normalize();
		ViewFrustum out;
		// Near
		out.m_Planes[0].SetPlane(camPos + camDir*nearPlane, camDir);
		// Far
		out.m_Planes[1].SetPlane(camPos + camDir*farPlane, -camDir);
		// Left
		out.m_Planes[2].SetPlane(camPos - camRight*xMax, camRight);
		// Right
		out.m_Planes[3].SetPlane(camPos + camRight*xMax, -camRight);
		// Top
		out.m_Planes[4].SetPlane(camPos + camUp*yMax, -camUp);
		// Bottom
		out.m_Planes[5].SetPlane(camPos - camUp*yMax, camUp);

		out.RecalculateInternals();
		return out;
	}

	//! Access the planes
	core::Range<const math::PlaneF*> Planes() const
	{
		return core::MakeRange(m_Planes, m_Planes + Count);
	}

	//! Access a single plane
	const math::PlaneF& Plane(EPlane p) const
	{
		lxAssert((int)p < Count);
		return m_Planes[(int)p];
	}

	//! Set the value of a plane
	/**
	Internal data is not updated \ref RecalculateInternals
	*/
	void SetPlane(EPlane p, const math::PlaneF& value)
	{
		lxAssert((int)p < Count);
		m_Planes[(int)p] = value;
		m_Planes[(int)p].Normalize();
	}

	//! Access the corners
	/**
	\param start The first corner to return, a total of 8-start corners are returned
	\return Range over corners
	*/
	core::Range<const math::Vector3F*> Corners(int start = 0) const
	{
		lxAssert(start >= 0 && start <= 8);
		return core::MakeRange(m_Points+start, m_Points+8);
	}

	//! Access a single corner
	const math::Vector3F& Corner(int id) const
	{
		lxAssert(id >= 0 && id < 8);
		return m_Points[id];
	}

	//! Recalculate all internal variables from the planes.
	void RecalculateInternals()
	{
		// Corners
		for(int i = 0; i < 8; ++i) {
			int a=0, b=0, c=0;
			switch(i) {
			case 0: a = Near; b = Left; c = Top; break;
			case 1: a = Near; b = Right; c = Top; break;
			case 2: a = Near; b = Right; c = Bottom; break;
			case 3: a = Near; b = Left; c = Bottom; break;
			case 4: a = Far; b = Left; c = Top; break;
			case 5: a = Far; b = Right; c = Top; break;
			case 6: a = Far; b = Right; c = Bottom; break;
			case 7: a = Far; b = Left; c = Bottom; break;
			}

			math::Vector3F point;
			bool intersect = m_Planes[a].IntersectWithPlanes(
				m_Planes[b],
				m_Planes[c], point);
			lxAssert(intersect);
			m_Points[i] = point;
		}
		
		// Bounding box
		m_BoundingBox.Set(Corner(0));
		for(auto c : Corners(1))
			m_BoundingBox.AddPoint(c);

	}

	//! Check if a point is inside the frustum
	/**
	Including the edge
	*/
	bool IsPointInside(const math::Vector3F& point) const
	{
		for(auto& p : m_Planes) {
			if(p.ClassifyPoint(point) == math::PlaneF::ERelation::Back)
				return false;
		}

		return true;
	}

	ERelation ClassifyBox(const math::AABBoxF& box) const
	{
		bool fullyInside = true;
		for(auto& p : m_Planes) {
			auto cls = p.ClassifyBox(box.minCorner, box.maxCorner);
			if(cls == math::PlaneF::ERelation::Back)
				return ERelation::Outside;
			if(cls == math::PlaneF::ERelation::Clipped)
				fullyInside = false;
		}

		return fullyInside ? ERelation::Inside : ERelation::Clipped;
	}

	//! Check if a axis-aligned box is visible in the frustum
	/**
	Including the edge
	*/
	bool IsBoxVisible(const math::AABBoxF& box) const
	{
		return ClassifyBox(box) != ERelation::Outside;
	}

	ERelation ClassifyBox(const math::AABBoxF& box, const math::Transformation& boxTransform) const
	{
		bool fullyInside = true;
		auto invTransform = boxTransform.GetInverted();
		for(auto& p : m_Planes) {
			auto transPlane = invTransform.TransformObject(p);
			auto cls = transPlane.ClassifyBox(box.minCorner, box.maxCorner);
			if(cls == math::PlaneF::ERelation::Back)
				return ERelation::Outside;
			if(cls == math::PlaneF::ERelation::Clipped)
				fullyInside = false;
		}

		return fullyInside ? ERelation::Inside : ERelation::Clipped;
	}

	//! Check if a object-oriented box is visible in the frustum
	/**
	Including the edge
	*/
	bool IsBoxVisible(const math::AABBoxF& box, const math::Transformation& boxTransform) const
	{
		return ClassifyBox(box, boxTransform) != ERelation::Outside;
	}

	//! Transform the box with a matrix
	void Transform(const math::Matrix4& m)
	{
		for(auto& p : m_Planes)
			p = m.TransformPlane(p);
	}

private:
	math::PlaneF m_Planes[6];
	math::Vector3F m_Points[8];
	math::AABBoxF m_BoundingBox;
};

} // namespace math
} // namespace lux

#endif // #ifndef INCLUDED_LUX_MATH_VIEW_FRUSTUM_H