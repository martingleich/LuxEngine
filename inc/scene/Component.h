#ifndef INCLUDED_LUX_SCENE_COMPONENT_H
#define INCLUDED_LUX_SCENE_COMPONENT_H
#include "core/Referable.h"
#include "scene/Renderable.h"

namespace lux
{
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

//! A scene node component
class Component : public core::Referable
{
	friend class Node;
public:
	class Listener
	{
	public:
		virtual ~Listener() {}

		virtual void Attach(const Node* node) { LUX_UNUSED(node); }
		virtual void Dettach(const Node* node) { LUX_UNUSED(node); }
	};

public:
	Component() :
		m_Node(nullptr),
		m_Listener(nullptr),
		m_IsAnimated(false)
	{
	}

	virtual ~Component() {}

	virtual void VisitRenderables(RenderableVisitor* visitor, ERenderableTags tags)
	{
		LUX_UNUSED(visitor);
		LUX_UNUSED(tags);
	}

	//! Called to animate the component
	/**
	This is called only if the component is animated
	\param time The time since the last frame in seconds
	*/
	virtual void Animate(float time)
	{
		LUX_UNUSED(time);
	}

	virtual void SetAnimated(bool animated)
	{
		m_IsAnimated = animated;
	}

	virtual void SetListener(Listener* l)
	{
		m_Listener = l;
	}

	virtual Listener* GetListener() const
	{
		return m_Listener;
	}

	bool IsAnimated() const
	{
		return m_IsAnimated;
	}

	Node* GetParent()
	{
		return m_Node;
	}
	const Node* GetParent() const
	{
		return m_Node;
	}

	StrongRef<Component> Clone() const
	{
		return CloneImpl().StaticCastStrong<Component>();
	}

protected:
	virtual void OnAttach(Node* n)
	{
		if(m_Listener)
			m_Listener->Attach(n);
		m_Node = n;
	}
	virtual void OnDetach(Node* n)
	{
		if(m_Listener)
			m_Listener->Dettach(n);
		m_Node = nullptr;
	}

protected:
	Node* m_Node;
	Listener* m_Listener;
	bool m_IsAnimated;
};

} // namespace scene
} // namespace lux

#endif