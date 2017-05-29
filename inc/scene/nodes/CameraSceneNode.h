#ifndef INCLUDED_CAMERASCENENODE_H
#define INCLUDED_CAMERASCENENODE_H
#include "scene/SceneNode.h"
#include "video/RenderTarget.h"

namespace lux
{
namespace scene
{

//! Represent a camera in the scenegraph
class CameraSceneNode : public SceneNode
{
public:
	CameraSceneNode();

	//! Get the current projection matrix.
	/**
	This may not reflect the currently set values.
	*/
	LUX_API const math::matrix4& GetProjection() const;

	LUX_API void SetCustomProjection(const math::matrix4& proj);
	LUX_API void ClearCustomProjection();

	//! Get the current view matrix.
	/**
	This may not reflect the currently set values.
	*/
	LUX_API const math::matrix4& GetView() const;

	LUX_API void SetViewModification(const math::matrix4& mod);
	LUX_API const math::matrix4& GetViewModification();

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

	//! Set the LUX_API root of the camera.
	/**
	This camera displays all nodes below and inclusive the LUX_API root.
	If set to null the whole scene is rendered.
	Default value is null.
	*/
	LUX_API void SetVirtualRoot(SceneNode* node);
	LUX_API SceneNode* GetVirtualRoot() const;

	LUX_API void SetRenderTarget(const video::RenderTarget& target);
	LUX_API const video::RenderTarget& GetRenderTarget() const;

	LUX_API void SetRenderPriority(s32 p);
	LUX_API s32 GetRenderPriority() const;

	LUX_API void SetBackgroundColor(video::Color color);
	LUX_API video::Color GetBackgroundColor();

	void Render();
	bool SetSceneManager(SceneManager* smgr);

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();

private:
	math::matrix4 m_Projection;
	bool m_HasCustomProjection;

	math::matrix4 m_View;
	math::matrix4 m_ViewModification;

	float m_FOV;
	float m_XMax;
	float m_Aspect;

	float m_NearPlane;
	float m_FarPlane;

	bool m_IsOrtho;

	video::RenderTarget m_RenderTarget;
	u32 m_RenderPriority;
	video::Color m_BackgroundColor;

	WeakRef<SceneNode> m_VirtualRoot;

	bool m_AutoAspect;
};

} // namespace scene
} // namespace lux

#endif