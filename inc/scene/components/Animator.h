#ifndef INCLUDED_LUX_ANIMATOR_H
#define INCLUDED_LUX_ANIMATOR_H
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
		SetAnimated(true);
	}

	virtual void Animate(float secsPassed) = 0;
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
		SetAnimated(other.IsAnimated());
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
	float m_StartTime;
	float m_RemainingTime;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ANIMATOR_H