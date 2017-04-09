#ifndef INCLUDED_SCENENODE_COMPONENT_H
#define INCLUDED_SCENENODE_COMPONENT_H
#include "core/Referable.h"
#include "input/EventReceiver.h"

namespace lux
{
namespace scene
{

class SceneNode;
namespace SceneNodeComponentType
{
LUX_API extern const core::Name Rotation;
LUX_API extern const core::Name CameraFPS;
LUX_API extern const core::Name LinearMove;
}

//! A scene node component
/**
An component can change all parameters of a scenenode.
For example position, materials, play animations.
An component can receive Userinput, usefull for i.e. Cameracontrol.
A component can add any further information to scene nodes.
*/
class SceneNodeComponent : public Referable, public input::EventReceiver
{
	friend class SceneNode;
public:
	SceneNodeComponent() :
		m_IsAnimated(false),
		m_IsActive(true),
		m_IsEventReceiver(false)
	{}

	virtual ~SceneNodeComponent()
	{}

	//! Get the scene node owning this component.
	WeakRef<SceneNode> GetParent() const
	{
		return m_Parent;
	}

	//! Is the component animated, meaning it will receive a Update function once per frame.
	bool IsAnimated() const
	{
		return m_IsAnimated;
	}

	//! Should the component receive userinput
	/**
	\return Should the component receive userinput
	\ref SetEventReceiver
	\ref OnEvent
	*/
	bool IsEventReceiver() const
	{
		return m_IsEventReceiver;
	}

	//! Should the component receive userinput
	/**
	The animator will only receive events if the scenenodes its attached to receive input.
	\param receiveEvents Should the component receive userinput
	\ref IsEventReceiver
	\ref OnEvent
	*/
	void SetEventReceiver(bool receiveEvents)
	{
		m_IsEventReceiver = receiveEvents;
	}

	//! Called if an engine-events occurs
	/**
	Only called if the component is set as eventreceiver
	\ref input::EventReceiver
	\ref SetEventReceiver
	*/
	virtual bool OnEvent(const input::Event& event)
	{
		LUX_UNUSED(event);
		return false;
	}

	//! Called to animate the component
	/**
	The transformation of the node is not updated.
	\param time The time since the last frame in seconds
	*/
	virtual void Animate(float time)
	{
		LUX_UNUSED(time);
	}

	//! Clone this component
	/**
	See \ref Referable
	\return The cloned scenenode
	*/
	virtual StrongRef<Referable> Clone() const
	{
		return nullptr;
	}

	//! The type of the component
	/**
	\return The type of the component
	*/
	virtual core::Name GetReferableSubType() const
	{
		return core::Name::INVALID;
	}

	//! Set the component active
	/**
	Only usefull if the component is animated.
	\param active Should the component be active
	\ref IsActive
	*/
	virtual void SetActive(bool active)
	{
		m_IsActive = active;
	}

	//! Is the component active
	/**
	\return Is the component active
	\ref SetActive
	*/
	bool IsActive() const
	{
		return m_IsActive;
	}

	core::Name GetReferableType() const
	{
		return ReferableType::SceneNodeComponent;
	}

protected:
	//! Flag this component as animated.
	/**
	User components can call this method in their constructor, to flag
	the component as animated.
	It's Animate method will the called once per frame.
	\param isAnimated Is the component animated
	*/
	void SetAnimated(bool isAnimated)
	{
		m_IsAnimated = isAnimated;
	}

private:
	//! Set the given node as new parent, without adding it, or removing it from the old one.
	LUX_API void SetParent(SceneNode* new_parent);

private:
	bool m_IsAnimated;
	bool m_IsActive;
	bool m_IsEventReceiver;
	WeakRef<SceneNode> m_Parent;
};

class AnimatedSceneNodeComponent : public SceneNodeComponent
{
public:
	AnimatedSceneNodeComponent()
	{
		SetAnimated(true);
	}
};

class FinishingSceneNodeComponent : public SceneNodeComponent
{
public:
	FinishingSceneNodeComponent(float timeToFinish = 0.0f) :
		m_RemainingTime(timeToFinish)
	{
		SetAnimated(true);
	}

	virtual void Animate(float secsPassed)
	{
		m_RemainingTime -= secsPassed;
		if(IsFinished())
			OnFinish();
	}

	virtual void OnFinish() {}

	bool IsFinished() const
	{
		return m_RemainingTime <= 0.0f;
	}

protected:
	float m_RemainingTime;
};

}    

}    


#endif