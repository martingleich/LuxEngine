#ifndef INCLUDED_CAMERASCENENODE_IMPL_H
#define INCLUDED_CAMERASCENENODE_IMPL_H
#include "scene/nodes/CameraSceneNode.h"
#include "video/RenderTarget.h"

namespace lux
{
namespace scene
{

class CameraSceneNodeImpl : public CameraSceneNode
{
public:
	CameraSceneNodeImpl();

	void SetCustomProjection(const math::matrix4& proj);
	void ClearCustomProjection();

	const math::matrix4& GetProjection() const;
	const math::matrix4& GetView() const;

	void SetViewModification(const math::matrix4& mod);
	const math::matrix4& GetViewModification();

	void SetAspect(float aspect);
	float GetAspect() const;

	void SetFOV(float fov);
	float GetFOV() const;

	void SetXMax(float xmax);
	float GetXMax() const;

	void SetNearPlane(float near);
	float GetNearPlane() const;

	void SetFarPlane(float far);
	float GetFarPlane() const;

	void SetOrtho(bool ortho);
	bool IsOrtho() const;

	void SetAutoAspect(bool automatic);
	bool GetAutoAspect();

	void SetRenderTarget(const video::RenderTarget& target);
	const video::RenderTarget& GetRenderTarget() const;

	void SetRenderPriority(s32 p);
	s32 GetRenderPriority() const;

	void SetBackgroundColor(video::Color color);
	video::Color GetBackgroundColor();

	void SetVirtualRoot(SceneNode* node);
	SceneNode* GetVirtualRoot() const;

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