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
	//! Get the current projection matrix.
	/**
	This may not reflect the currently set values.
	*/
	virtual const math::matrix4& GetProjection() const = 0;

	virtual void SetCustomProjection(const math::matrix4& proj) = 0;
	virtual void ClearCustomProjection() = 0;

	//! Get the current view matrix.
	/**
	This may not reflect the currently set values.
	*/
	virtual const math::matrix4& GetView() const = 0;

	virtual void SetViewModification(const math::matrix4& mod) = 0;
	virtual const math::matrix4& GetViewModification() = 0;

	virtual void SetAspect(float aspect) = 0;
	virtual float GetAspect() const = 0;

	virtual void SetXMax(float xmax) = 0;
	virtual float GetXMax() const = 0;

	virtual void SetFOV(float fov) = 0;
	virtual float GetFOV() const = 0;

	virtual void SetNearPlane(float near) = 0;
	virtual float GetNearPlane() const = 0;

	virtual void SetFarPlane(float far) = 0;
	virtual float GetFarPlane() const = 0;

	//! Set if the camera is a orthgonal camera.
	/**
	Default value is false.
	*/
	virtual void SetOrtho(bool ortho) = 0;
	//! Is the camera orthogonal
	virtual bool IsOrtho() const = 0;

	//! The aspect ratio is automatic calculated from the rendertarget.
	/**
	Default value is true.
	*/
	virtual void SetAutoAspect(bool automatic) = 0;
	//! Is the aspect ratio automatic calculated.
	virtual bool GetAutoAspect() = 0;

	//! Set the virtual root of the camera.
	/**
	This camera displays all nodes below and inclusive the virtual root.
	If set to null the whole scene is rendered.
	Default value is null.
	*/
	virtual void SetVirtualRoot(SceneNode* node) = 0;
	virtual SceneNode* GetVirtualRoot() const = 0;

	virtual void SetRenderTarget(const video::RenderTarget& target) = 0;
	virtual const video::RenderTarget& GetRenderTarget() const = 0;

	virtual void SetRenderPriority(s32 p) = 0;
	virtual s32 GetRenderPriority() const = 0;

	virtual void SetBackgroundColor(video::Color color) = 0;
	virtual video::Color GetBackgroundColor() = 0;
};

} // namespace scene
} // namespace lux

#endif