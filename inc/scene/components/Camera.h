#ifndef INCLUDED_LUX_CAMERASCENENODE_H
#define INCLUDED_LUX_CAMERASCENENODE_H
#include "scene/Component.h"
#include "video/RenderTarget.h"
#include "math/Matrix4.h"
#include "math/ViewFrustum.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

class AbstractCamera : public Component
{
public:
	class Listener
	{
	public:
		//! Called before rendering the camera, after beginning the scene.
		virtual void PreRender(AbstractCamera* cam) { LUX_UNUSED(cam); }

		//! Called after rendering all nodes, before ending the scene.
		virtual void PostRender(AbstractCamera* cam) { LUX_UNUSED(cam); }
	};

public:
	virtual void PreRender(video::Renderer* renderer) = 0;
	virtual void Render(video::Renderer* renderer) = 0;
	virtual void PostRender(video::Renderer* renderer) = 0;

	virtual void SetRenderTarget(const video::RenderTarget& target) = 0;
	virtual const video::RenderTarget& GetRenderTarget() const = 0;

	virtual void SetRenderPriority(s32 p) = 0;
	virtual s32 GetRenderPriority() const = 0;

	virtual const math::ViewFrustum& GetActiveFrustum() const = 0;

	virtual void SetCameraListener(Listener* l) = 0;
	virtual Listener* GetCameraListener() const = 0;

	StrongRef<AbstractCamera> Clone()
	{
		return CloneImpl().StaticCastStrong<AbstractCamera>();
	}
};

//! Represent a camera in the scenegraph
class Camera : public AbstractCamera
{
	LX_REFERABLE_MEMBERS_API(Camera, LUX_API);
public:
	LUX_API Camera();
	LUX_API ~Camera();

	LUX_API void SetCustomProjection(const math::Matrix4& proj, const math::ViewFrustum& frustum);
	LUX_API const math::Matrix4& GetCustomProjection();
	LUX_API void ClearCustomProjection();

	LUX_API void SetViewModification(const math::Matrix4& mod);
	LUX_API const math::Matrix4& GetViewModification();

	//! Screen width divided by Screen height
	LUX_API void SetAspect(float aspect);
	LUX_API float GetAspect() const;

	LUX_API void SetXMax(float xmax);
	LUX_API float GetXMax() const;

	//! Set vertical field of vision in rad
	LUX_API void SetFOV(math::AngleF fovY);
	//! Get vertical field of vision in rad
	LUX_API math::AngleF GetFOV() const;

	LUX_API void SetNearPlane(float near);
	LUX_API float GetNearPlane() const;

	LUX_API void SetFarPlane(float far);
	LUX_API float GetFarPlane() const;

	//! Set if the camera is a orthgonal camera.
	/**
	Default value is false.
	*/
	LUX_API void SetOrtho(bool ortho);
	//! Is the camera orthogonal
	LUX_API bool IsOrtho() const;

	//! The aspect ratio is automatic calculated from the rendertarget.
	/**
	Default value is true.
	*/
	LUX_API void SetAutoAspect(bool automatic);
	//! Is the aspect ratio automatic calculated.
	LUX_API bool GetAutoAspect();

	LUX_API void SetRenderTarget(const video::RenderTarget& target);
	LUX_API const video::RenderTarget& GetRenderTarget() const;

	LUX_API void SetRenderPriority(int p);
	LUX_API int GetRenderPriority() const;

	LUX_API void SetCameraListener(Listener* l);
	LUX_API Listener* GetCameraListener() const;

	LUX_API void PreRender(video::Renderer* renderer);
	LUX_API void Render(video::Renderer* r);
	LUX_API void PostRender(video::Renderer* renderer);

	LUX_API const math::ViewFrustum& GetActiveFrustum() const;
	LUX_API const math::Matrix4& GetActiveView() const;
	LUX_API const math::Matrix4& GetActiveProjection() const;

private:
	math::Matrix4 CalculateProjectionMatrix(video::Renderer* r);
	math::Matrix4 CalculateViewMatrix(video::Renderer* r);
	math::ViewFrustum CalculateViewFrustum(video::Renderer* r, const math::Matrix4& view);

private:
	math::ViewFrustum m_CustomFrustum;
	math::Matrix4 m_CustomProjection;
	bool m_HasCustomProjection;

	math::Matrix4 m_ViewModification;

	math::AngleF m_FOV;
	float m_XMax;
	float m_Aspect;

	float m_NearPlane;
	float m_FarPlane;

	bool m_IsOrtho;

	video::RenderTarget m_RenderTarget;
	int m_RenderPriority;

	Listener* m_Listener;

	math::Matrix4 m_ActiveProjection;
	math::Matrix4 m_ActiveView;
	math::ViewFrustum m_ActiveViewFrustum;

	bool m_AutoAspect;
};

} // namespace scene
} // namespace lux

#endif