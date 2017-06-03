#ifndef INCLUDED_ANIMATOR_H
#define INCLUDED_ANIMATOR_H
#include "scene/Component.h"

namespace lux
{
namespace scene
{

class Animator : public Component
{
public:
	Animator()
	{
		m_IsAnimated = true;
	}

	virtual void Animate(Node* node, float secsPassed) = 0;
};

class FinishingAnimator : public Animator
{
public:
	FinishingAnimator(float timeToFinish = 0.0f) :
		m_StartTime(timeToFinish),
		m_RemainingTime(timeToFinish)
	{
	}

	FinishingAnimator(const FinishingAnimator& other) :
		m_StartTime(other.m_StartTime),
		m_RemainingTime(other.m_StartTime)
	{
		m_IsAnimated = other.m_IsAnimated;
	}

	virtual void Animate(Node* node, float secsPassed)
	{
		m_RemainingTime -= secsPassed;
		if(IsFinished())
			OnFinish(node);
	}

	virtual void OnFinish(Node* node)
	{
		LUX_UNUSED(node);
	}

	bool IsFinished() const
	{
		return m_RemainingTime <= 0.0f;
	}

protected:
	float m_StartTime;
	float m_RemainingTime;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_ANIMATOR_H