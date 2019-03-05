#ifndef INCLUDED_LUX_SCENE_COMPONENT_H
#define INCLUDED_LUX_SCENE_COMPONENT_H
#include "core/Referable.h"
#include "math/AABBox.h"
#include "scene/SceneRendererData.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

namespace SceneComponentType
{
LUX_API extern const core::Name Rotation;
LUX_API extern const core::Name FirstPersonCameraControl;
LUX_API extern const core::Name TurntableCameraControl;
LUX_API extern const core::Name LinearMove;

LUX_API extern const core::Name Camera;
LUX_API extern const core::Name Mesh;
LUX_API extern const core::Name PointLight;
LUX_API extern const core::Name SpotLight;
LUX_API extern const core::Name DirLight;
LUX_API extern const core::Name LinearFog;
LUX_API extern const core::Name ExponentialFog;
LUX_API extern const core::Name SkyBox;
}

class Scene;

//! A scene node component
class Component : public core::Referable
{
	friend class Node;
public:
	Component() :
		m_Node(nullptr),
		m_IsAnimated(false)
	{
	}
	~Component()
	{
	}
	Component(const Component& other) :
		m_Node(nullptr),
		m_IsAnimated(other.m_IsAnimated)
	{
	}

	virtual void Render(const SceneRenderData&) {}
	virtual RenderPassSet GetRenderPass() const { return RenderPassSet(); }

	virtual const math::AABBoxF& GetBoundingBox() const { return math::AABBoxF::EMPTY; }

	//! Called to animate the component
	/**
	This is called only if the component is animated
	\param time The time since the last frame in seconds
	*/
	virtual void Animate(float time) { LUX_UNUSED(time); }

	Node* GetNode() { return m_Node; }
	LUX_API Scene* GetScene();

	StrongRef<Component> Clone() const { return CloneImpl().StaticCastStrong<Component>(); }

	LUX_API virtual void Register(bool doRegister);

	bool IsAnimated() const { return m_IsAnimated; }

protected:
	LUX_API void SetAnimated(bool animated);

protected:
	void OnAttach(Node* n) { m_Node = n; }
	void OnDetach(Node*) { m_Node = nullptr; }

private:
	Node* m_Node;
	bool m_IsAnimated;
};

} // namespace scene
} // namespace lux

#endif