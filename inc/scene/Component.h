#ifndef INCLUDED_SCENE_COMPONENT_H
#define INCLUDED_SCENE_COMPONENT_H
#include "core/Referable.h"

namespace lux
{
namespace scene
{

class Node;
class Renderable;
class RenderableVisitor;

namespace SceneComponentType
{
LUX_API extern const core::Name Rotation;
LUX_API extern const core::Name CameraControl;
LUX_API extern const core::Name LinearMove;

LUX_API extern const core::Name Camera;
LUX_API extern const core::Name Mesh;
LUX_API extern const core::Name Light;
LUX_API extern const core::Name SkyBox;
}

//! A scene node component
class Component : public Referable
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
		m_IsAnimated(false),
		m_Listener(nullptr)
	{
	}

	virtual ~Component() {}

	virtual void VisitRenderables(RenderableVisitor* visitor, bool noDebug)
	{
		LUX_UNUSED(visitor);
		LUX_UNUSED(noDebug);
	}

	//! Called to animate the component
	/**
	This is called only if the component is animated
	\param time The time since the last frame in seconds
	*/
	virtual void Animate(Node* node, float time)
	{
		LUX_UNUSED(node);
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

	//! Clone this component
	/**
	See \ref Referable
	\return The cloned scenenode
	*/
	virtual StrongRef<Referable> Clone() const
	{
		throw core::NotImplementedException();
	}

	//! The type of the component
	/**
	\return The type of the component
	*/
	virtual core::Name GetReferableSubType() const
	{
		return core::Name::INVALID;
	}

	core::Name GetReferableType() const
	{
		return ReferableType::SceneNodeComponent;
	}

protected:
	virtual void OnAttach(Node* n)
	{
		if(m_Listener)
			m_Listener->Attach(n);
	}

	virtual void OnDettach(Node* n)
	{
		if(m_Listener)
			m_Listener->Dettach(n);
	}

protected:
	bool m_IsAnimated;
	Listener* m_Listener;
};

} // namespace scene
} // namespace lux

#endif