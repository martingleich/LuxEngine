#ifndef INCLUDED_CAMERASCENENODE_H
#define INCLUDED_CAMERASCENENODE_H
#include "scene/Component.h"
#include "video/RenderTarget.h"
#include "math/Matrix4.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

class CameraBase : public Component
{
public:
	class Listener
	{
	public:
		//! Called before rendering the camera, after beginning the scene.
		virtual void PreRender(const CameraBase* cam, const Node* n) { LUX_UNUSED(cam); LUX_UNUSED(n); }

		//! Called after rendering all nodes, before ending the scene.
		virtual void PostRender(const CameraBase* cam, const Node* n) { LUX_UNUSED(cam); LUX_UNUSED(n); }
	};

public:
	virtual void PreRender(video::Renderer* renderer, const Node* node) = 0;
	virtual void Render(video::Renderer* renderer, const Node* node) = 0;
	virtual void PostRender(video::Renderer* renderer, const Node* node) = 0;

	virtual void SetRenderTarget(const video::RenderTarget& target) = 0;
	virtual const video::RenderTarget& GetRenderTarget() const = 0;

	virtual void SetRenderPriority(s32 p) = 0;
	virtual s32 GetRenderPriority() const = 0;

	virtual void SetCameraListener(Listener* l) = 0;
	virtual Listener* GetCameraListener() const = 0;
};

//! Represent a camera in the scenegraph
class Camera : public CameraBase
{
public:
	LUX_API Camera();
	LUX_API ~Camera();

	LUX_API void SetCustomProjection(const math::Matrix4& proj);
	LUX_API const math::Matrix4& GetCustomProjection();
	LUX_API void ClearCustomProjection();

	LUX_API void SetViewModification(const math::Matrix4& mod);
	LUX_API const math::Matrix4& GetViewModification();

	LUX_API void SetAspect(float aspect);
	LUX_API float GetAspect() const;

	LUX_API void SetXMax(float xmax);
	LUX_API float GetXMax() const;

	LUX_API void SetFOV(float fov);
	LUX_API float GetFOV() const;

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

	LUX_API void SetRenderPriority(s32 p);
	LUX_API s32 GetRenderPriority() const;

	LUX_API void SetCameraListener(Listener* l);
	LUX_API Listener* GetCameraListener() const;

	LUX_API void PreRender(video::Renderer* renderer, const Node* node);
	LUX_API void Render(video::Renderer* r, const Node* n);
	LUX_API void PostRender(video::Renderer* renderer, const Node* node);

	LUX_API core::Name GetReferableType() const;
	LUX_API StrongRef<Referable> Clone() const;

private:
	math::Matrix4 CalculateProjectionMatrix(video::Renderer* r, const Node* n);
	math::Matrix4 CalculateViewMatrix(video::Renderer* r, const Node* n);

private:
	math::Matrix4 m_CustomProjection;
	bool m_HasCustomProjection;

	math::Matrix4 m_ViewModification;

	float m_FOV;
	float m_XMax;
	float m_Aspect;

	float m_NearPlane;
	float m_FarPlane;

	bool m_IsOrtho;

	video::RenderTarget m_RenderTarget;
	u32 m_RenderPriority;

	Listener* m_Listener;

	bool m_AutoAspect;
};

} // namespace scene
} // namespace lux

#endif