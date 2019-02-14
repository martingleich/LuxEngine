#ifndef INCLUDED_LUX_CAMERASCENENODE_H
#define INCLUDED_LUX_CAMERASCENENODE_H
#include "scene/Component.h"
#include "video/RenderTarget.h"
#include "math/Matrix4.h"
#include "math/ViewFrustum.h"

namespace lux
{
namespace scene
{

//! Base class for simple cameras.
class BaseCamera : public Component
{
public:
	class Controller : public SceneRenderPassController
	{
	public:
		Controller(BaseCamera* cam) :
			m_Cam(cam)
		{
		}
		int GetPriority() const { return m_Cam->GetRenderPriority(); }
		void Render(SceneRenderPassHelper* helper);
	
	public:
		BaseCamera* m_Cam;
	};

	class Listener
	{
	public:
		//! Called before rendering the camera, after beginning the scene.
		virtual void PreRender(BaseCamera* cam) { LUX_UNUSED(cam); }

		//! Called after rendering all nodes, before ending the scene.
		virtual void PostRender(BaseCamera* cam) { LUX_UNUSED(cam); }
	};

public:
	LUX_API BaseCamera();

	//! Screen width divided by Screen height
	void SetAspect(float aspect) { m_Aspect = aspect; }
	float GetAspect() const { return m_Aspect; }

	void SetNearPlane(float near) { m_NearPlane = near; }
	float GetNearPlane() const { return m_NearPlane; }

	void SetFarPlane(float far) { m_FarPlane = far; }
	float GetFarPlane() const { return m_FarPlane; }

	void SetRenderTarget(const video::RenderTarget& target) { m_RenderTarget = target; }
	const video::RenderTarget& GetRenderTarget() const { return m_RenderTarget; }

	void SetRenderPriority(int p) { m_RenderPriority = p; }
	int GetRenderPriority() const { return m_RenderPriority; }

	void SetCameraListener(Listener* l) { m_Listener = l; }
	Listener* GetCameraListener() const { return m_Listener; }

	LUX_API void Register(bool doRegister) override;

protected:
	virtual void CalculateCamData(math::Matrix4& view, math::Matrix4& proj, math::ViewFrustum& frustum) = 0;

protected:
	float m_Aspect;

	float m_NearPlane;
	float m_FarPlane;

	video::RenderTarget m_RenderTarget;
	int m_RenderPriority;

	Listener* m_Listener;

	Controller m_Controller;
};

//! Represent a perspective camera in the scenegraph
class PerspCamera : public BaseCamera
{
	LX_REFERABLE_MEMBERS_API(PerspCamera, LUX_API);
public:
	LUX_API PerspCamera();
	LUX_API ~PerspCamera();

	//! Set vertical field of vision.
	void SetFOV(math::AngleF fovY) { m_FOV = fovY; }
	//! Get vertical field of vision.
	math::AngleF GetFOV() const { return m_FOV; }

private:
	void CalculateCamData(math::Matrix4& view, math::Matrix4& proj, math::ViewFrustum& frustum) override;

protected:
	math::AngleF m_FOV;
};

} // namespace scene
} // namespace lux

#endif