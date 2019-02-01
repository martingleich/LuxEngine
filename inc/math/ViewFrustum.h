#ifndef INCLUDED_LUX_MATH_VIEW_FRUSTUM_H
#define INCLUDED_LUX_MATH_VIEW_FRUSTUM_H
#include "math/Plane.h"
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
	}

	//! Create view frustum from perspective camera
	/**
	\param view The view matrix used by the camera
	\param fovY The vertical field of view in rad.
	\param aspect The aspect ratio(screenWidth/screenHeight)
	\param nearPlane The distance of the near clipping plane
	\param farPlane The distance of the far clipping plane
	\return The generated view frustum
	*/
	static ViewFrustum FromPerspCam(
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
		auto camPos = -view(3, 0) * camRight - view(3, 1) * camUp - view(3, 2) * camDir;

		ViewFrustum out;
		out.m_Planes[Near].SetPlane(camPos + camDir * nearPlane, camDir);
		out.m_Planes[Far].SetPlane(camPos + camDir * farPlane, -camDir);
		
		float thorz = math::Tan(fovY*0.5f)*aspect;
		auto leftSide = camDir - thorz * camRight;
		auto leftNormal = camUp.Cross(leftSide).Normal();
		out.m_Planes[Left].SetPlane(camPos, leftNormal);
		
		auto rightSide = camDir + thorz * camRight;
		auto rightNormal = rightSide.Cross(camUp).Normal();
		out.m_Planes[Right].SetPlane(camPos, rightNormal);
		
		float tvert = math::Tan(fovY*0.5f);
		auto topSide = camDir + tvert * camUp;
		auto topNormal = camRight.Cross(topSide).Normal();
		out.m_Planes[Top].SetPlane(camPos, topNormal);
		
		auto bottomSide = camDir - tvert * camUp;
		auto bottomNormal = bottomSide.Cross(camRight).Normal();
		out.m_Planes[Bottom].SetPlane(camPos, bottomNormal);

		return out;
	}

	//! Create view frustum from orthogonal camera
	/**
	\param view The view matrix used by the camera
	\param xMax the maximal visible x offset
	\param aspect The aspect ratio(screenWidth/screenHeight)
	\param nearPlane The distance of the near clipping plane
	\param farPlane The distance of the far clipping plane
	\return The generated view frustum
	*/
	static ViewFrustum FromOrthoCam(
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
		auto camPos = -view(3, 0) * camRight - view(3, 1) * camUp - view(3, 2) * camDir;

		ViewFrustum out;
		out.m_Planes[Near].SetPlane(camPos + camDir*nearPlane, camDir);
		out.m_Planes[Far].SetPlane(camPos + camDir*farPlane, -camDir);
		out.m_Planes[Left].SetPlane(camPos - camRight*xMax, camRight);
		out.m_Planes[Right].SetPlane(camPos + camRight*xMax, -camRight);
		out.m_Planes[Top].SetPlane(camPos + camUp*yMax, -camUp);
		out.m_Planes[Bottom].SetPlane(camPos - camUp*yMax, camUp);

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
	void SetPlane(EPlane p, const math::PlaneF& value)
	{
		lxAssert((int)p < Count);
		m_Planes[(int)p] = value;
		m_Planes[(int)p].Normalize();
	}

private:
	math::PlaneF m_Planes[6];
};

} // namespace math
} // namespace lux

#endif // #ifndef INCLUDED_LUX_MATH_VIEW_FRUSTUM_H